@echo off
qmake -project -t lib "TARGET = AuClientMailRes" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUCLIENTMAILRES" "RC_FILE = AuClientMailRes.rc"
qmake
