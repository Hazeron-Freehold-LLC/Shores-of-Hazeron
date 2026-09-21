@echo off
qmake -project -t lib "TARGET = AuModelRes" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUMODELRES" "RC_FILE = AuModelRes.rc"
qmake "LIBS += ../release/AuModel1.lib ../release/AuModel2.lib ../release/AuModel3.lib ../release/AuModel4.lib ../release/AuModel5.lib ../release/AuModel6.lib ../release/AuModel7.lib ../release/AuModel8.lib ../release/AuModel9.lib ../release/AuModel10.lib"
