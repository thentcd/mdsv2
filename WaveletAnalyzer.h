#ifndef WAVELETANALYZER_H
#define WAVELETANALYZER_H

#include <QtWidgets>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>
#include <vector>
#include <complex>
#include <memory>


class SignalPlotWidget;
class ScalogramWidget;

class WaveletAnalyzer : public QMainWindow
{
    Q_OBJECT

public:
    explicit WaveletAnalyzer(QWidget *parent = nullptr);
    ~WaveletAnalyzer();

private slots:
    void loadSignalFile();
    void selectChannel(int channel);
    void setSignalParameters();
    void selectWavelet(int waveletType);
    void setScaleParameters();
    void setTimeRange();
    void performCWT();
    void resetView();

private:
    void setupUI();
    void setupMenuBar();
    void setupSignalControls();
    void setupWaveletControls();
    void setupVisualization();
    void updateSignalInfo();
    void updatePlots();
    
    // Data structures
    struct SignalData {
        std::vector<std::vector<double>> channels;
        std::vector<double> timeVector;
        double samplingRate;
        int selectedChannel;
        QString filename;
    };
    
    struct CWTParameters {
        int waveletType; 
        int minScale;
        int maxScale;
        int scaleSteps;
        int startSample;
        int endSample;
    };
    
    
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QSplitter *m_plotSplitter;
    
    
    QGroupBox *m_signalGroup;
    QGroupBox *m_waveletGroup;
    QGroupBox *m_analysisGroup;
    
    
    QPushButton *m_loadButton;
    QLabel *m_fileLabel;
    QComboBox *m_channelCombo;
    QSpinBox *m_samplesSpinBox;
    QDoubleSpinBox *m_samplingRateSpinBox;
    QSlider *m_startSlider;
    QSlider *m_endSlider;
    QLabel *m_rangeLabel;
    
    
    QComboBox *m_waveletCombo;
    QSpinBox *m_minScaleSpinBox;
    QSpinBox *m_maxScaleSpinBox;
    QSpinBox *m_scaleStepsSpinBox;
    QPushButton *m_analyzeButton;
    QPushButton *m_resetButton;
    
    
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QTextEdit *m_infoTextEdit;
    
    
    SignalPlotWidget *m_signalPlot;
    ScalogramWidget *m_scalogramPlot;
    
    
    SignalData m_signalData;
    CWTParameters m_cwtParams;
    std::vector<std::vector<std::complex<double>>> m_cwtCoefficients;
    std::vector<double> m_scales;
    
    
    bool loadCSVFile(const QString &filename);
    void parseCSVLine(const QString &line, std::vector<double> &values);
    std::vector<std::complex<double>> computeCWT(const std::vector<double> &signal, 
                                                 const std::vector<double> &scales,
                                                 int waveletType);
    std::complex<double> morletWavelet(double t, double scale);
    std::complex<double> mexicanHatWavelet(double t, double scale);
    std::complex<double> daubechiesWavelet(double t, double scale);
};


class SignalPlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SignalPlotWidget(QWidget *parent = nullptr);
    void setSignalData(const std::vector<double> &signal, const std::vector<double> &time);
    void setTimeRange(int start, int end);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::vector<double> m_signal;
    std::vector<double> m_time;
    int m_startIndex;
    int m_endIndex;
    double m_zoomFactor;
    QPoint m_lastPanPoint;
    
    void drawSignal(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter);
};

class ScalogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScalogramWidget(QWidget *parent = nullptr);
    void setCWTData(const std::vector<std::vector<std::complex<double>>> &coefficients,
                    const std::vector<double> &scales,
                    const std::vector<double> &time);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    std::vector<std::vector<std::complex<double>>> m_coefficients;
    std::vector<double> m_scales;
    std::vector<double> m_time;
    QImage m_scalogramImage;
    
    void generateScalogramImage();
    QColor valueToColor(double magnitude, double maxMagnitude);
    void drawColorScale(QPainter &painter);
};

#endif 