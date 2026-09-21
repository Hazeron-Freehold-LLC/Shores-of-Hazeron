@echo off
qmake -project -t lib "TARGET = AuMath" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "DESTDIR = ../release" "DEFINES += AUMATH" "QMAKE_CXXFLAGS += /MP"
qmake
