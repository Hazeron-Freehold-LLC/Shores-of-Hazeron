@echo off
qmake -project -t lib "TARGET = AuModel6" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "QT += opengl network sql" "DESTDIR = ../Release" "QMAKE_CXXFLAGS_RELEASE += -Zm1500"
qmake
