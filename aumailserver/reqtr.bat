@echo off
qmake -project "TARGET = AuMailServer" "VERSION = 7.0" "TEMPLATE = app" "CONFIG += qt thread" "QT += opengl network sql" "INCLUDEPATH = ../auutil ../auatom ../audata ../auglobal ../aumailserver ../ausceneserver ../augl ../aumath" "DESTDIR = ../Release"
qmake "LIBS += ../release/auutil7.lib ../release/AuGL.lib ../release/AuMath.lib ../release/AuData.lib ../release/AuGlobal.lib"
