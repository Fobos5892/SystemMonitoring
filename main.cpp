#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include "Data/metatypes.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<SensorData>();
    qRegisterMetaType<SensorStatistics>();
    qRegisterMetaType<QVector<SensorData>>();
    qRegisterMetaType<DBDataControll::AnchorSide>();

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/assets/app-icon.svg"));
    MainWindow w;
    w.show();
    return QApplication::exec();
}
