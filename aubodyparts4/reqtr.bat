@echo off
qmake -project -t lib "TARGET = AuBodyParts4" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "QT += opengl network sql" "DESTDIR = ../Release"
qmake
