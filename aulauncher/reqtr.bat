@echo off
qmake -project "TARGET = AuLauncher" "VERSION = 7.0" "TEMPLATE = app" "CONFIG += qt thread" "QT += network widgets" "DESTDIR = ../Release" "RC_FILE = AuLauncher.rc" "INCLUDEPATH = ..\auutil"
qmake "LIBS += ../release/auutil.lib"
