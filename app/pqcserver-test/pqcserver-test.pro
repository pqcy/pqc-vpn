QT -= gui
include(../../pqc-vpn.pri)
TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
INCLUDEPATH += ../../src
INCLUDEPATH += ../../../openssl/include ../../../openssl/oqs/include
LIBS += -L../../../openssl -lssl -lcrypto
DESTDIR=../../bin
SOURCES += pqcserver-test.cpp
