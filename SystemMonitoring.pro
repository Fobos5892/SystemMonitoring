QT += core gui widgets sql svg

CONFIG += c++20 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Application/Coordination/threadorchestrator.cpp \
    Application/telemetryfacade.cpp \
    Domain/sensorstatistics.cpp \
    Domain/filterqueryspec.cpp \
    Domain/telemetrymergelogic.cpp \
    Infrastructure/Devices/devicereceiver.cpp \
    Infrastructure/Devices/devicesimulator.cpp \
    Infrastructure/Persistence/dbconnect.cpp \
    Infrastructure/Persistence/dbdatacontroll.cpp \
    Infrastructure/Persistence/dbtelemetryrepository.cpp \
    View/main.cpp \
    View/mainwindow.cpp \
    ViewModels/filterviewmodel.cpp \
    ViewModels/statisticsviewmodel.cpp \
    ViewModels/telemetrytablemodel.cpp \
    ViewModels/telemetryviewmodel.cpp

HEADERS += \
    Application/Contracts/itelemetryrepository.h \
    Application/Coordination/threadorchestrator.h \
    Application/telemetryfacade.h \
    Domain/metatypes.h \
    Domain/datetimeformats.h \
    Domain/filterlimits.h \
    Domain/sensorlimits.h \
    Domain/sensordata.h \
    Domain/sensorstatistics.h \
    Domain/filterqueryspec.h \
    Domain/telemetrymergelogic.h \
    Domain/telemetrytypes.h \
    Infrastructure/Devices/devicereceiver.h \
    Infrastructure/Devices/devicesimulator.h \
    Infrastructure/Persistence/dbconnect.h \
    Infrastructure/Persistence/dbdatacontroll.h \
    Infrastructure/Persistence/dbtelemetryrepository.h \
    View/mainwindow.h \
    ViewModels/filterviewmodel.h \
    ViewModels/statisticsviewmodel.h \
    ViewModels/telemetrytablemodel.h \
    ViewModels/telemetryviewmodel.h

FORMS += \
    View/mainwindow.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
