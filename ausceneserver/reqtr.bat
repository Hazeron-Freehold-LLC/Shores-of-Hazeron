@echo off
qmake -project "TARGET = AuSceneServer" "VERSION = 7.0" "TEMPLATE = app" "CONFIG += qt thread" "QT += opengl network sql" "INCLUDEPATH = ../auatom ../audata ../auglobal ../auloginclient ../aumail ../augl ../aumath ../auatomres ../aubodypartsres ../aumodelres" "DESTDIR = ../Release"
qmake "LIBS += ../release/AuAtom.lib ../release/AuData.lib ../release/AuGlobal.lib ../release/AuUtil.lib ../release/AuGL.lib ../release/AuLoginClient.lib ../release/AuMail.lib ../release/AuAtomRes.lib ../release/AuBodyPartsRes.lib ../release/AuModelRes.lib"
