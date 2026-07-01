#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QVector>
#include "Data/sensordata.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<QVector<SensorData>>("QVector<SensorData>");

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/assets/app-icon.svg"));
    MainWindow w;
    w.show();
    return QApplication::exec();
}
