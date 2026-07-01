TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    app \
    tests

app.file = SystemMonitoring.pro
tests.file = tests/tests.pro
tests.depends = app
