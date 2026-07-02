QT += core testlib sql

CONFIG += console c++20 warn_on depend_includepath
CONFIG -= app_bundle

TARGET = SystemMonitoringTests
TEMPLATE = app

ROOT = ..

INCLUDEPATH += $$ROOT

SOURCES += \
    $$ROOT/Domain/sensorstatistics.cpp \
    $$ROOT/Domain/filterqueryspec.cpp \
    $$ROOT/Infrastructure/Persistence/dbconnect.cpp \
    $$ROOT/Infrastructure/Persistence/dbdatacontroll.cpp \
    $$ROOT/ViewModels/filterviewmodel.cpp \
    $$ROOT/ViewModels/statisticsviewmodel.cpp \
    $$ROOT/ViewModels/telemetrytablemodel.cpp \
    $$ROOT/ViewModels/telemetryviewmodel.cpp \
    tst_main.cpp \
    tst_sensorstatistics.cpp \
    tst_filterqueryspec.cpp \
    tst_filterviewmodel.cpp \
    tst_statisticsviewmodel.cpp \
    tst_telemetryviewmodel.cpp \
    tst_dbdatacontroll.cpp

HEADERS += \
    tst_sensorstatistics.h \
    tst_filterqueryspec.h \
    tst_filterviewmodel.h \
    tst_statisticsviewmodel.h \
    tst_telemetryviewmodel.h \
    tst_dbdatacontroll.h \
    $$ROOT/Domain/telemetrytypes.h \
    $$ROOT/Domain/datetimeformats.h \
    $$ROOT/Domain/filterlimits.h \
    $$ROOT/Domain/sensorlimits.h \
    $$ROOT/Domain/filterqueryspec.h \
    $$ROOT/ViewModels/filterviewmodel.h \
    $$ROOT/ViewModels/statisticsviewmodel.h \
    $$ROOT/ViewModels/telemetrytablemodel.h \
    $$ROOT/ViewModels/telemetryviewmodel.h \
    $$ROOT/Infrastructure/Persistence/dbconnect.h \
    $$ROOT/Infrastructure/Persistence/dbdatacontroll.h

# Сборка: открыть tests/tests.pro в Qt Creator или:
#   cd tests && qmake && mingw32-make && release/SystemMonitoringTests.exe
