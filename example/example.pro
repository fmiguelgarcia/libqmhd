TEMPLATE = app
QT       = core network
CONFIG  += c++11
TARGET   = example

INCLUDEPATH += $$PWD/../src
LIBS        += -L$$OUT_PWD/../src -lqmhd

include(../conanbuildinfo.pri)
CONFIG += conan_basic_setup

INCLUDEPATH += $$CONAN_INCLUDEPATH
LIBS += $$CONAN_LIBS


SOURCES += example.cpp
HEADERS += example.h
