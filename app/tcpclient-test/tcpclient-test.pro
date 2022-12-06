QT -= gui
include(../../pqc-vpn.pri)
TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
INCLUDEPATH += ../../src
DESTDIR=../../bin
SOURCES += tcpclient-test.cpp
