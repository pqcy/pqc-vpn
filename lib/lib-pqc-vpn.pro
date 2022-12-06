QT -= gui
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++17
CONFIG += K_BUILD
DEFINES += K_BUILD
include(../pqc-vpn.pri)
TARGET = $${K_NAME}
DESTDIR = $${PWD}/../bin
include(lib-pqc-vpn-files.pri)
