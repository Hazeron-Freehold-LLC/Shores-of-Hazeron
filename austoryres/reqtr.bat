@echo off
qmake -project -t lib "TARGET = AuStoryRes" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUSTORYRES" "RC_FILE = AuStoryRes.rc"
qmake
