#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include "Domain/metatypes.h"

int main(int argc, char *argv[])
{
    registerDomainMetaTypes();

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/assets/app-icon.svg"));
    MainWindow w;
    w.show();
    return QApplication::exec();
}
