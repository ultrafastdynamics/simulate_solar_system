//QT
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QString>

#include "qt_window.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLocale::setDefault(QLocale::English);
    std::setlocale(LC_ALL, "en_US.UTF-8");

    QFile file("Ubuntu.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());

    app.setStyleSheet(styleSheet);
    QTWindow window;

    window.show();
    return app.exec();
}
