QT -= gui
include(../../pqc-vpn.pri)
TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
INCLUDEPATH += ../../src
LIBS += -lssl -lcrypto
DESTDIR=../../bin
SOURCES += tlsserver-test.cpp
