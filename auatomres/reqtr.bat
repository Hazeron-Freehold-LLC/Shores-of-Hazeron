@echo off
qmake -project -t lib "TARGET = AuAtomRes" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUATOMRES" "RC_FILE = AuAtomRes.rc"
qmake "LIBS += ../release/AuAtomR1.lib ../release/AuAtomR2.lib ../release/AuAtomR3.lib ../release/AuAtomR4.lib ../release/AuAtomR5.lib ../release/AuAtomR6.lib"
