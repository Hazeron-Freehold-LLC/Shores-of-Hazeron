@echo off
qmake -project -t lib "TARGET = AuAtomR4" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "DESTDIR = ../Release"
qmake
