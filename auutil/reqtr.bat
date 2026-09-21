@echo off
qmake -project -t lib "TARGET = AuUtil" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "QT = network sql widgets" "DESTDIR = ..\release" "DEFINES += AUUTIL" "INCLUDEPATH = ..\auloginserver ..\aumailserver" "QMAKE_CXXFLAGS += /MP"
qmake
