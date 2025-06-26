#include <QtWidgets/QApplication>
#include "WaveletAnalyzer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("CWT Scalogram Analyzer");
    app.setApplicationVersion("1.0");
    
    WaveletAnalyzer analyzer;
    analyzer.show();
    
    return app.exec();
}