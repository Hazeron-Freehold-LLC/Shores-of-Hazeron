@echo off
qmake -project -t lib "TARGET = AuClientRes" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUCLIENTRES" "RC_FILE = AuClientRes.rc"
qmake "LIBS += ../release/AuClientRes1.lib"
