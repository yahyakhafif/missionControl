#include <QApplication>
#include <QPalette>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setStyle("Fusion");

    QPalette pal;
    pal.setColor(QPalette::Window,          QColor(10,  12,  18));
    pal.setColor(QPalette::WindowText,      QColor(120, 170, 140));
    pal.setColor(QPalette::Base,            QColor( 8,  11,  16));
    pal.setColor(QPalette::AlternateBase,   QColor(13,  16,  24));
    pal.setColor(QPalette::ToolTipBase,     QColor(13,  16,  24));
    pal.setColor(QPalette::ToolTipText,     QColor(120, 170, 140));
    pal.setColor(QPalette::Text,            QColor(120, 170, 140));
    pal.setColor(QPalette::Button,          QColor(15,  24,  32));
    pal.setColor(QPalette::ButtonText,      QColor(120, 170, 140));
    pal.setColor(QPalette::BrightText,      QColor(  0, 245, 160));
    pal.setColor(QPalette::Link,            QColor(  0, 200, 130));
    pal.setColor(QPalette::Highlight,       QColor( 13,  50,  35));
    pal.setColor(QPalette::HighlightedText, QColor(  0, 245, 160));
    app.setPalette(pal);

    MainWindow w;
    w.show();

    return app.exec();
}
