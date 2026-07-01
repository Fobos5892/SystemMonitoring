QT += core gui widgets sql svg

CONFIG += c++20 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Data/sensorstatistics.cpp \
    DBModel/dbconnect.cpp \
    DBModel/dbdatacontroll.cpp \
    DeviceReceiver/devicereceiver.cpp \
    DeviceSimulator/devicesimulator.cpp \
    ThreadOrchestrator/threadorchestrator.cpp \
    ViewModels/sensormodel.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Data/metatypes.h \
    Data/sensordata.h \
    Data/sensorstatistics.h \
    DBModel/dbconnect.h \
    DBModel/dbdatacontroll.h \
    DeviceReceiver/devicereceiver.h \
    DeviceSimulator/devicesimulator.h \
    ThreadOrchestrator/threadorchestrator.h \
    ViewModels/sensormodel.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
