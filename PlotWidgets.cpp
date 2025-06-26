#include "WaveletAnalyzer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>


SignalPlotWidget::SignalPlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_startIndex(0)
    , m_endIndex(1000)
    , m_zoomFactor(1.0)
{
    setMinimumHeight(200);
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void SignalPlotWidget::setSignalData(const std::vector<double> &signal, const std::vector<double> &time)
{
    m_signal = signal;
    m_time = time;
    m_endIndex = std::min(static_cast<int>(signal.size()), 1000);
    update();
}

void SignalPlotWidget::setTimeRange(int start, int end)
{
    m_startIndex = std::max(0, start);
    m_endIndex = std::min(static_cast<int>(m_signal.size()), end);
    if (m_startIndex >= m_endIndex) {
        m_endIndex = m_startIndex + 1;
    }
    update();
}

void SignalPlotWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    
    if (m_signal.empty() || m_time.empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "No signal data");
        return;
    }
    
    drawGrid(painter);
    drawAxes(painter);
    drawSignal(painter);
}

void SignalPlotWidget::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    
    const int margin = 50;
    const QRect plotArea(margin, margin, width() - 2 * margin, height() - 2 * margin);
    
    
    for (int i = 0; i <= 10; ++i) {
        int x = plotArea.left() + i * plotArea.width() / 10;
        painter.drawLine(x, plotArea.top(), x, plotArea.bottom());
    }
    
    
    for (int i = 0; i <= 5; ++i) {
        int y = plotArea.top() + i * plotArea.height() / 5;
        painter.drawLine(plotArea.left(), y, plotArea.right(), y);
    }
}

void SignalPlotWidget::drawAxes(QPainter &painter)
{
    painter.setPen(QPen(Qt::black, 2));
    
    const int margin = 50;
    const QRect plotArea(margin, margin, width() - 2 * margin, height() - 2 * margin);
    
    
    painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight());
    
    
    painter.drawLine(plotArea.bottomLeft(), plotArea.topLeft());
    
    
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10));
    
    if (m_startIndex < m_time.size() && m_endIndex <= m_time.size()) {
        
        for (int i = 0; i <= 5; ++i) {
            int x = plotArea.left() + i * plotArea.width() / 5;
            int timeIdx = m_startIndex + i * (m_endIndex - m_startIndex) / 5;
            if (timeIdx < m_time.size()) {
                QString label = QString::number(m_time[timeIdx], 'f', 3);
                painter.drawText(x - 25, plotArea.bottom() + 20, 50, 20, 
                               Qt::AlignCenter, label);
            }
        }
        
        
        auto minMax = std::minmax_element(m_signal.begin() + m_startIndex, 
                                         m_signal.begin() + m_endIndex);
        double minVal = *minMax.first;
        double maxVal = *minMax.second;
        double range = maxVal - minVal;
        
        for (int i = 0; i <= 5; ++i) {
            int y = plotArea.bottom() - i * plotArea.height() / 5;
            double value = minVal + i * range / 5;
            QString label = QString::number(value, 'f', 2);
            painter.drawText(5, y - 10, 40, 20, Qt::AlignRight | Qt::AlignVCenter, label);
        }
    }
    
    
    painter.save();
    painter.translate(15, plotArea.center().y());
    painter.rotate(-90);
    painter.drawText(-50, -5, 100, 20, Qt::AlignCenter, "Amplitude");
    painter.restore();
    
    painter.drawText(plotArea.center().x() - 25, height() - 10, 50, 20, 
                    Qt::AlignCenter, "Time (s)");
}

void SignalPlotWidget::drawSignal(QPainter &painter)
{
    if (m_startIndex >= m_endIndex || m_endIndex > m_signal.size()) {
        return;
    }
    
    const int margin = 50;
    const QRect plotArea(margin, margin, width() - 2 * margin, height() - 2 * margin);
    
    painter.setPen(QPen(Qt::blue, 2));
    painter.setRenderHint(QPainter::Antialiasing);
    
    
    auto minMax = std::minmax_element(m_signal.begin() + m_startIndex, 
                                     m_signal.begin() + m_endIndex);
    double minVal = *minMax.first;
    double maxVal = *minMax.second;
    double range = maxVal - minVal;
    
    if (range < 1e-10) {
        range = 1.0; 
    }
    
    QPolygonF points;
    int numSamples = m_endIndex - m_startIndex;
    
    for (int i = 0; i < numSamples; ++i) {
        int signalIdx = m_startIndex + i;
        if (signalIdx < m_signal.size()) {
            double x = plotArea.left() + (double)i * plotArea.width() / (numSamples - 1);
            double normalizedY = (m_signal[signalIdx] - minVal) / range;
            double y = plotArea.bottom() - normalizedY * plotArea.height();
            points << QPointF(x, y);
        }
    }
    
    if (points.size() > 1) {
        painter.drawPolyline(points);
    }
}

void SignalPlotWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPanPoint = event->pos();
}

void SignalPlotWidget::wheelEvent(QWheelEvent *event)
{
    
    double scaleFactor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    m_zoomFactor *= scaleFactor;
    
    
    m_zoomFactor = qBound(0.1, m_zoomFactor, 10.0);
    
    update();
}


ScalogramWidget::ScalogramWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(300);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void ScalogramWidget::setCWTData(const std::vector<std::vector<std::complex<double>>> &coefficients,
                                 const std::vector<double> &scales,
                                 const std::vector<double> &time)
{
    m_coefficients = coefficients;
    m_scales = scales;
    m_time = time;
    
    if (!coefficients.empty()) {
        generateScalogramImage();
    }
    
    update();
}

void ScalogramWidget::generateScalogramImage()
{
    if (m_coefficients.empty() || m_scales.empty() || m_time.empty()) {
        return;
    }
    
    int timeSteps = m_coefficients[0].size();
    int scaleSteps = m_coefficients.size();
    
    m_scalogramImage = QImage(timeSteps, scaleSteps, QImage::Format_RGB32);
    
    
    double maxMagnitude = 0.0;
    for (const auto &scaleCoeffs : m_coefficients) {
        for (const auto &coeff : scaleCoeffs) {
            maxMagnitude = std::max(maxMagnitude, std::abs(coeff));
        }
    }
    
    if (maxMagnitude < 1e-10) {
        maxMagnitude = 1.0;
    }
    
    /
    for (int scaleIdx = 0; scaleIdx < scaleSteps; ++scaleIdx) {
        for (int timeIdx = 0; timeIdx < timeSteps; ++timeIdx) {
            double magnitude = std::abs(m_coefficients[scaleIdx][timeIdx]);
            QColor color = valueToColor(magnitude, maxMagnitude);
            
            
            m_scalogramImage.setPixelColor(timeIdx, scaleSteps - 1 - scaleIdx, color);
        }
    }
}

QColor ScalogramWidget::valueToColor(double magnitude, double maxMagnitude)
{
    double normalized = magnitude / maxMagnitude;
    normalized = qBound(0.0, normalized, 1.0);
    
    
    if (normalized < 0.25) {
        
        double t = normalized / 0.25;
        return QColor(0, static_cast<int>(255 * t), 255);
    } else if (normalized < 0.5) {
        
        double t = (normalized - 0.25) / 0.25;
        return QColor(0, 255, static_cast<int>(255 * (1 - t)));
    } else if (normalized < 0.75) {
        
        double t = (normalized - 0.5) / 0.25;
        return QColor(static_cast<int>(255 * t), 255, 0);
    } else {
        
        double t = (normalized - 0.75) / 0.25;
        return QColor(255, static_cast<int>(255 * (1 - t)), 0);
    }
}

void ScalogramWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    
    if (m_scalogramImage.isNull()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "No CWT data - perform analysis first");
        return;
    }
    
    const int margin = 50;
    const QRect plotArea(margin, margin, width() - 100, height() - 2 * margin);
    
    
    painter.drawImage(plotArea, m_scalogramImage);
    
    
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10));
    
    
    painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight());
    for (int i = 0; i <= 5; ++i) {
        int x = plotArea.left() + i * plotArea.width() / 5;
        painter.drawLine(x, plotArea.bottom(), x, plotArea.bottom() + 5);
        
        if (i < m_time.size()) {
            int timeIdx = i * (m_time.size() - 1) / 5;
            QString label = QString::number(m_time[timeIdx], 'f', 3);
            painter.drawText(x - 25, plotArea.bottom() + 10, 50, 20, 
                           Qt::AlignCenter, label);
        }
    }
    
    
    painter.drawLine(plotArea.bottomLeft(), plotArea.topLeft());
    for (int i = 0; i <= 5; ++i) {
        int y = plotArea.bottom() - i * plotArea.height() / 5;
        painter.drawLine(plotArea.left() - 5, y, plotArea.left(), y);
        
        if (i < m_scales.size()) {
            int scaleIdx = i * (m_scales.size() - 1) / 5;
            QString label = QString::number(m_scales[scaleIdx], 'f', 1);
            painter.drawText(5, y - 10, 40, 20, Qt::AlignRight | Qt::AlignVCenter, label);
        }
    }
    
    
    drawColorScale(painter);
    
    
    painter.save();
    painter.translate(15, plotArea.center().y());
    painter.rotate(-90);
    painter.drawText(-30, -5, 60, 20, Qt::AlignCenter, "Scale");
    painter.restore();
    
    painter.drawText(plotArea.center().x() - 25, height() - 10, 50, 20, 
                    Qt::AlignCenter, "Time (s)");
    
    painter.drawText(plotArea.right() + 10, 20, 80, 20, 
                    Qt::AlignLeft, "Magnitude");
}

void ScalogramWidget::drawColorScale(QPainter &painter)
{
    const int margin = 50;
    const QRect colorScale(width() - 80, margin, 20, height() - 2 * margin);
    
    
    for (int y = 0; y < colorScale.height(); ++y) {
        double normalized = 1.0 - static_cast<double>(y) / colorScale.height();
        QColor color = valueToColor(normalized, 1.0);
        painter.fillRect(colorScale.x(), colorScale.y() + y, colorScale.width(), 1, color);
    }
    
    
    painter.setPen(Qt::black);
    painter.drawRect(colorScale);
    
    
    painter.setFont(QFont("Arial", 8));
    for (int i = 0; i <= 5; ++i) {
        int y = colorScale.bottom() - i * colorScale.height() / 5;
        double value = static_cast<double>(i) / 5;
        QString label = QString::number(value, 'f', 1);
        painter.drawText(colorScale.right() + 5, y - 5, 30, 10, 
                        Qt::AlignLeft | Qt::AlignVCenter, label);
    }
}

void ScalogramWidget::mousePressEvent(QMouseEvent *event)
{
    
    QWidget::mousePressEvent(event);
}