#include "WaveletAnalyzer.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QtMath>
#include <QDebug>
#include <algorithm>
#include <fftw3.h>

WaveletAnalyzer::WaveletAnalyzer(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_signalPlot(nullptr)
    , m_scalogramPlot(nullptr)
{
    setupUI();
    setupMenuBar();
    
    
    m_cwtParams.waveletType = 0; 
    m_cwtParams.minScale = 1;
    m_cwtParams.maxScale = 64;
    m_cwtParams.scaleSteps = 64;
    m_cwtParams.startSample = 0;
    m_cwtParams.endSample = 1000;
    
    m_signalData.samplingRate = 1000.0;
    m_signalData.selectedChannel = 0;
    
    setWindowTitle("CWT Scalogram Analyzer");
    resize(1200, 800);
}

WaveletAnalyzer::~WaveletAnalyzer() = default;

void WaveletAnalyzer::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    
    auto *mainLayout = new QHBoxLayout(m_centralWidget);
    
    
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(m_mainSplitter);
    
    
    setupSignalControls();
    setupWaveletControls();
    
    
    auto *controlsWidget = new QWidget;
    auto *controlsLayout = new QVBoxLayout(controlsWidget);
    controlsLayout->addWidget(m_signalGroup);
    controlsLayout->addWidget(m_waveletGroup);
    controlsLayout->addWidget(m_analysisGroup);
    controlsLayout->addStretch();
    
    
    setupVisualization();
    
    
    m_mainSplitter->addWidget(controlsWidget);
    m_mainSplitter->addWidget(m_plotSplitter);
    m_mainSplitter->setSizes({300, 900});
}

void WaveletAnalyzer::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    
    auto *loadAction = new QAction("&Load Signal", this);
    loadAction->setShortcut(QKeySequence::Open);
    connect(loadAction, &QAction::triggered, this, &WaveletAnalyzer::loadSignalFile);
    fileMenu->addAction(loadAction);
    
    fileMenu->addSeparator();
    
    auto *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    auto *helpMenu = menuBar()->addMenu("&Help");
    auto *aboutAction = new QAction("&About", this);
    
    
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About CWT Analyzer",
                          "CWT Scalogram Analyzer v1.0\n\n"
                          "Continuous Wavelet Transform analysis tool\n"
                          "for multi-channel signal processing.\n\n"
                          "Built with Qt5 for Lubuntu 20.04");
    });
    
    helpMenu->addAction(aboutAction);
}
void WaveletAnalyzer::setupSignalControls()
{
    m_signalGroup = new QGroupBox("Signal Parameters");
    auto *layout = new QGridLayout(m_signalGroup);
    
    
    m_loadButton = new QPushButton("Load Signal File");
    m_fileLabel = new QLabel("No file loaded");
    m_fileLabel->setWordWrap(true);
    
    layout->addWidget(m_loadButton, 0, 0, 1, 2);
    layout->addWidget(new QLabel("File:"), 1, 0);
    layout->addWidget(m_fileLabel, 1, 1);
    
    
    layout->addWidget(new QLabel("Channel:"), 2, 0);
    m_channelCombo = new QComboBox;
    layout->addWidget(m_channelCombo, 2, 1);
    
    
    layout->addWidget(new QLabel("Samples:"), 3, 0);
    m_samplesSpinBox = new QSpinBox;
    m_samplesSpinBox->setRange(1, 1000000);
    m_samplesSpinBox->setValue(1000);
    layout->addWidget(m_samplesSpinBox, 3, 1);
    
    layout->addWidget(new QLabel("Sampling Rate (Hz):"), 4, 0);
    m_samplingRateSpinBox = new QDoubleSpinBox;
    m_samplingRateSpinBox->setRange(1.0, 100000.0);
    m_samplingRateSpinBox->setValue(1000.0);
    m_samplingRateSpinBox->setSuffix(" Hz");
    layout->addWidget(m_samplingRateSpinBox, 4, 1);
    
    
    layout->addWidget(new QLabel("Start Sample:"), 5, 0);
    m_startSlider = new QSlider(Qt::Horizontal);
    m_startSlider->setRange(0, 1000);
    layout->addWidget(m_startSlider, 5, 1);
    
    layout->addWidget(new QLabel("End Sample:"), 6, 0);
    m_endSlider = new QSlider(Qt::Horizontal);
    m_endSlider->setRange(0, 1000);
    m_endSlider->setValue(1000);
    layout->addWidget(m_endSlider, 6, 1);
    
    m_rangeLabel = new QLabel("Range: 0 - 1000");
    layout->addWidget(m_rangeLabel, 7, 0, 1, 2);
    
    
    connect(m_loadButton, &QPushButton::clicked, this, &WaveletAnalyzer::loadSignalFile);
    connect(m_channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WaveletAnalyzer::selectChannel);
    connect(m_samplesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &WaveletAnalyzer::setSignalParameters);
    connect(m_samplingRateSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &WaveletAnalyzer::setSignalParameters);
    connect(m_startSlider, &QSlider::valueChanged, this, &WaveletAnalyzer::setTimeRange);
    connect(m_endSlider, &QSlider::valueChanged, this, &WaveletAnalyzer::setTimeRange);
}

void WaveletAnalyzer::setupWaveletControls()
{
    m_waveletGroup = new QGroupBox("Wavelet Parameters");
    auto *layout = new QGridLayout(m_waveletGroup);
    
    
    layout->addWidget(new QLabel("Wavelet Type:"), 0, 0);
    m_waveletCombo = new QComboBox;
    m_waveletCombo->addItems({"Morlet", "Mexican Hat", "Daubechies"});
    layout->addWidget(m_waveletCombo, 0, 1);
    
    
    layout->addWidget(new QLabel("Min Scale:"), 1, 0);
    m_minScaleSpinBox = new QSpinBox;
    m_minScaleSpinBox->setRange(1, 100);
    m_minScaleSpinBox->setValue(1);
    layout->addWidget(m_minScaleSpinBox, 1, 1);
    
    layout->addWidget(new QLabel("Max Scale:"), 2, 0);
    m_maxScaleSpinBox = new QSpinBox;
    m_maxScaleSpinBox->setRange(2, 512);
    m_maxScaleSpinBox->setValue(64);
    layout->addWidget(m_maxScaleSpinBox, 2, 1);
    
    layout->addWidget(new QLabel("Scale Steps:"), 3, 0);
    m_scaleStepsSpinBox = new QSpinBox;
    m_scaleStepsSpinBox->setRange(10, 256);
    m_scaleStepsSpinBox->setValue(64);
    layout->addWidget(m_scaleStepsSpinBox, 3, 1);
    
    
    connect(m_waveletCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WaveletAnalyzer::selectWavelet);
    connect(m_minScaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &WaveletAnalyzer::setScaleParameters);
    connect(m_maxScaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &WaveletAnalyzer::setScaleParameters);
    connect(m_scaleStepsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &WaveletAnalyzer::setScaleParameters);
}

void WaveletAnalyzer::setupVisualization()
{
    
    m_analysisGroup = new QGroupBox("Analysis");
    auto *analysisLayout = new QVBoxLayout(m_analysisGroup);
    
    m_analyzeButton = new QPushButton("Perform CWT Analysis");
    m_resetButton = new QPushButton("Reset View");
    
    m_progressBar = new QProgressBar;
    m_statusLabel = new QLabel("Ready");
    
    m_infoTextEdit = new QTextEdit;
    m_infoTextEdit->setMaximumHeight(100);
    m_infoTextEdit->setReadOnly(true);
    
    analysisLayout->addWidget(m_analyzeButton);
    analysisLayout->addWidget(m_resetButton);
    analysisLayout->addWidget(m_progressBar);
    analysisLayout->addWidget(m_statusLabel);
    analysisLayout->addWidget(new QLabel("Analysis Info:"));
    analysisLayout->addWidget(m_infoTextEdit);
    
    
    m_plotSplitter = new QSplitter(Qt::Vertical);
    
    m_signalPlot = new SignalPlotWidget;
    m_scalogramPlot = new ScalogramWidget;
    
    m_plotSplitter->addWidget(m_signalPlot);
    m_plotSplitter->addWidget(m_scalogramPlot);
    m_plotSplitter->setSizes({300, 500});
    
    
    connect(m_analyzeButton, &QPushButton::clicked, this, &WaveletAnalyzer::performCWT);
    connect(m_resetButton, &QPushButton::clicked, this, &WaveletAnalyzer::resetView);
}

void WaveletAnalyzer::loadSignalFile()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Load Signal File",
        "",
        "CSV Files (*.csv);;All Files (*)"
    );
    
    if (!filename.isEmpty()) {
        if (loadCSVFile(filename)) {
            m_fileLabel->setText(QFileInfo(filename).fileName());
            updateSignalInfo();
            updatePlots();
            m_statusLabel->setText("Signal loaded successfully");
        } else {
            QMessageBox::warning(this, "Error", "Failed to load signal file");
            m_statusLabel->setText("Failed to load signal");
        }
    }
}

bool WaveletAnalyzer::loadCSVFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    m_signalData.channels.clear();
    m_signalData.timeVector.clear();
    m_signalData.filename = filename;
    
    QTextStream in(&file);
    std::vector<std::vector<double>> tempData;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        std::vector<double> values;
        parseCSVLine(line, values);
        
        if (!values.empty()) {
            tempData.push_back(values);
        }
    }
    
    if (tempData.empty()) {
        return false;
    }
    
    
    int numChannels = tempData[0].size();
    if (numChannels < 2) {
        return false; 
    }
    
    
    m_signalData.channels.resize(numChannels - 1);
    
    for (const auto &row : tempData) {
        if (row.size() == numChannels) {
            m_signalData.timeVector.push_back(row[0]); 
            
            for (int ch = 1; ch < numChannels; ++ch) {
                m_signalData.channels[ch - 1].push_back(row[ch]);
            }
        }
    }
    
    
    if (m_signalData.timeVector.size() > 1) {
        double dt = m_signalData.timeVector[1] - m_signalData.timeVector[0];
        m_signalData.samplingRate = 1.0 / dt;
    }
    
    m_signalData.selectedChannel = 0;
    return true;
}

void WaveletAnalyzer::parseCSVLine(const QString &line, std::vector<double> &values)
{
    QStringList parts = line.split(';');
    if (parts.isEmpty()) {
        parts = line.split(',');
    }
    
    values.clear();
    for (const QString &part : parts) {
        bool ok;
        double value = part.trimmed().toDouble(&ok);
        if (ok) {
            values.push_back(value);
        }
    }
}

void WaveletAnalyzer::updateSignalInfo()
{
    if (m_signalData.channels.empty()) {
        return;
    }
    
    
    m_channelCombo->clear();
    for (int i = 0; i < m_signalData.channels.size(); ++i) {
        m_channelCombo->addItem(QString("Channel %1").arg(i + 1));
    }
    
    
    int numSamples = m_signalData.channels[0].size();
    m_samplesSpinBox->setValue(numSamples);
    m_samplingRateSpinBox->setValue(m_signalData.samplingRate);
    
    
    m_startSlider->setRange(0, numSamples - 1);
    m_endSlider->setRange(1, numSamples);
    m_startSlider->setValue(0);
    m_endSlider->setValue(numSamples);
    
    setTimeRange();
}

void WaveletAnalyzer::updatePlots()
{
    if (m_signalData.channels.empty() || m_signalData.selectedChannel >= m_signalData.channels.size()) {
        return;
    }
    
    const auto &signal = m_signalData.channels[m_signalData.selectedChannel];
    m_signalPlot->setSignalData(signal, m_signalData.timeVector);
    m_signalPlot->setTimeRange(m_cwtParams.startSample, m_cwtParams.endSample);
}


void WaveletAnalyzer::selectChannel(int channel)
{
    m_signalData.selectedChannel = channel;
    updatePlots();
    m_statusLabel->setText(QString("Selected channel %1").arg(channel + 1));
}

void WaveletAnalyzer::setSignalParameters()
{
    m_signalData.samplingRate = m_samplingRateSpinBox->value();
    updatePlots();
}

void WaveletAnalyzer::selectWavelet(int waveletType)
{
    m_cwtParams.waveletType = waveletType;
    QString waveletName = m_waveletCombo->currentText();
    m_statusLabel->setText(QString("Selected %1 wavelet").arg(waveletName));
}

void WaveletAnalyzer::setScaleParameters()
{
    m_cwtParams.minScale = m_minScaleSpinBox->value();
    m_cwtParams.maxScale = m_maxScaleSpinBox->value();
    m_cwtParams.scaleSteps = m_scaleStepsSpinBox->value();
    
    if (m_cwtParams.minScale >= m_cwtParams.maxScale) {
        m_maxScaleSpinBox->setValue(m_cwtParams.minScale + 1);
        m_cwtParams.maxScale = m_cwtParams.minScale + 1;
    }
}

void WaveletAnalyzer::setTimeRange()
{
    m_cwtParams.startSample = m_startSlider->value();
    m_cwtParams.endSample = m_endSlider->value();
    
    if (m_cwtParams.startSample >= m_cwtParams.endSample) {
        m_endSlider->setValue(m_cwtParams.startSample + 1);
        m_cwtParams.endSample = m_cwtParams.startSample + 1;
    }
    
    m_rangeLabel->setText(QString("Range: %1 - %2")
                         .arg(m_cwtParams.startSample)
                         .arg(m_cwtParams.endSample));
    
    updatePlots();
}

void WaveletAnalyzer::performCWT()
{
    if (m_signalData.channels.empty() || m_signalData.selectedChannel >= m_signalData.channels.size()) {
        QMessageBox::warning(this, "Error", "No signal data loaded");
        return;
    }
    
    m_analyzeButton->setEnabled(false);
    m_progressBar->setValue(0);
    m_statusLabel->setText("Performing CWT analysis...");
    
    QApplication::processEvents();
    
    try {
        
        const auto &fullSignal = m_signalData.channels[m_signalData.selectedChannel];
        std::vector<double> signal(fullSignal.begin() + m_cwtParams.startSample,
                                  fullSignal.begin() + m_cwtParams.endSample);
        
        
        m_scales.clear();
        double scaleStep = static_cast<double>(m_cwtParams.maxScale - m_cwtParams.minScale) 
                          / (m_cwtParams.scaleSteps - 1);
        
        for (int i = 0; i < m_cwtParams.scaleSteps; ++i) {
            m_scales.push_back(m_cwtParams.minScale + i * scaleStep);
        }
        
        
        m_cwtCoefficients = computeCWT(signal, m_scales, m_cwtParams.waveletType);
        
        
        std::vector<double> timeSegment(m_signalData.timeVector.begin() + m_cwtParams.startSample,
                                       m_signalData.timeVector.begin() + m_cwtParams.endSample);
        
        m_scalogramPlot->setCWTData(m_cwtCoefficients, m_scales, timeSegment);
        
        
        QString info = QString("CWT Analysis Complete\n"
                              "Wavelet: %1\n"
                              "Scales: %2 - %3 (%4 steps)\n"
                              "Samples: %5 - %6\n"
                              "Duration: %7 ms")
                      .arg(m_waveletCombo->currentText())
                      .arg(m_cwtParams.minScale)
                      .arg(m_cwtParams.maxScale)
                      .arg(m_cwtParams.scaleSteps)
                      .arg(m_cwtParams.startSample)
                      .arg(m_cwtParams.endSample)
                      .arg((m_cwtParams.endSample - m_cwtParams.startSample) * 1000.0 / m_signalData.samplingRate, 0, 'f', 2);
        
        m_infoTextEdit->setText(info);
        m_progressBar->setValue(100);
        m_statusLabel->setText("CWT analysis completed");
        
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", QString("CWT analysis failed: %1").arg(e.what()));
        m_statusLabel->setText("CWT analysis failed");
    }
    
    m_analyzeButton->setEnabled(true);
}

void WaveletAnalyzer::resetView()
{
    if (!m_signalData.channels.empty()) {
        m_startSlider->setValue(0);
        m_endSlider->setValue(m_signalData.channels[0].size());
        setTimeRange();
    }
    
    m_cwtCoefficients.clear();
    m_scales.clear();
    m_scalogramPlot->setCWTData({}, {}, {});
    m_infoTextEdit->clear();
    m_progressBar->setValue(0);
    m_statusLabel->setText("View reset");
}

std::vector<std::vector<std::complex<double>>> WaveletAnalyzer::computeCWT(
    const std::vector<double> &signal, 
    const std::vector<double> &scales,
    int waveletType)
{
    std::vector<std::vector<std::complex<double>>> coefficients;
    coefficients.resize(scales.size());
    
    int signalLength = signal.size();
    
    for (size_t scaleIdx = 0; scaleIdx < scales.size(); ++scaleIdx) {
        double scale = scales[scaleIdx];
        coefficients[scaleIdx].resize(signalLength);
        
        
        for (int t = 0; t < signalLength; ++t) {
            std::complex<double> coeff(0.0, 0.0);
            
            
            for (int tau = 0; tau < signalLength; ++tau) {
                double time = (tau - t) / scale;
                std::complex<double> waveletValue;
                
                switch (waveletType) {
                    case 0: waveletValue = morletWavelet(time, scale); break;
                    case 1: waveletValue = mexicanHatWavelet(time, scale); break;
                    case 2: waveletValue = daubechiesWavelet(time, scale); break;
                    default: waveletValue = morletWavelet(time, scale); break;
                }
                
                coeff += signal[tau] * std::conj(waveletValue);
            }
            
            coefficients[scaleIdx][t] = coeff / std::sqrt(scale);
        }
        
        
        int progress = static_cast<int>((scaleIdx + 1) * 100 / scales.size());
        m_progressBar->setValue(progress);
        QApplication::processEvents();
    }
    
    return coefficients;
}

std::complex<double> WaveletAnalyzer::morletWavelet(double t, double scale)
{
    const double sigma = 1.0;
    const double omega0 = 5.0; 
    
    double envelope = std::exp(-t * t / (2 * sigma * sigma));
    double oscillation_real = std::cos(omega0 * t);
    double oscillation_imag = std::sin(omega0 * t);
    
    return std::complex<double>(envelope * oscillation_real, envelope * oscillation_imag);
}

std::complex<double> WaveletAnalyzer::mexicanHatWavelet(double t, double scale)
{
    const double sigma = 1.0;
    double t_norm = t / sigma;
    double envelope = std::exp(-t_norm * t_norm / 2.0);
    double poly = 1.0 - t_norm * t_norm;
    
    return std::complex<double>(envelope * poly, 0.0);
}

std::complex<double> WaveletAnalyzer::daubechiesWavelet(double t, double scale)
{
    
    if (std::abs(t) > 3.0) {
        return std::complex<double>(0.0, 0.0);
    }
    
    const std::vector<double> coeffs = {
        0.6830127, 1.1830127, 0.3169873, -0.1830127
    };
    
    double value = 0.0;
    for (size_t i = 0; i < coeffs.size(); ++i) {
        double x = t + i - 1.5;
        if (std::abs(x) <= 0.5) {
            value += coeffs[i];
        }
    }
    
    return std::complex<double>(value, 0.0);
}

#include "WaveletAnalyzer.moc"