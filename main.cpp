#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QIcon>
#include <QDebug>
#include "WaveletAnalyzer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    
    app.setApplicationName("mdsv2");
    app.setApplicationVersion("2.0");
    app.setApplicationDisplayName("mdsv2");
    app.setOrganizationName("SignalProcessing");
    
    
    qDebug() << "Available icon theme paths:";
    QStringList iconPaths = QIcon::themeSearchPaths();
    for (const QString &path : iconPaths) {
        qDebug() << "  " << path;
        QDir dir(path);
        if (dir.exists()) {
            QStringList themes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &theme : themes) {
                qDebug() << "    Theme:" << theme;
            }
        }
    }
    
    
    QStringList availableThemes = {"hicolor", "Adwaita", "breeze", "oxygen", "Tango"};
    QString workingTheme;
    
    for (const QString &theme : availableThemes) {
        if (QIcon::hasThemeIcon("application-exit") || 
            QDir("/usr/share/icons/" + theme).exists()) {
            QIcon::setThemeName(theme);
            workingTheme = theme;
            qDebug() << "Using icon theme:" << theme;
            break;
        }
    }
    
    if (workingTheme.isEmpty()) {
        qDebug() << "No suitable icon theme found, using fallback";
        QIcon::setThemeName("hicolor"); 
    }
    
    
    QStringList availableStyles = QStyleFactory::keys();
    qDebug() << "Available styles:" << availableStyles;
    
    
    if (availableStyles.contains("Fusion")) {
        app.setStyle("Fusion");
        qDebug() << "Using Fusion style";
    } else if (availableStyles.contains("Windows")) {
        app.setStyle("Windows");
        qDebug() << "Using Windows style";
    }
    
    
    QPalette palette = app.palette();
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    app.setPalette(palette);
    
    
    try {
        WaveletAnalyzer analyzer;
        analyzer.show();
        
        qDebug() << "Application started successfully";
        return app.exec();
        
    } catch (const std::exception &e) {
        qDebug() << "Error creating main window:" << e.what();
        return 1;
    } catch (...) {
        qDebug() << "Unknown error creating main window";
        return 1;
    }
}