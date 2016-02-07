TEMPLATE = lib
VERSION = 3.0.0
CONFIG -= qt
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic
LIBS += -lpcap
LIBS += -L../lib -lmiil_core

DEPENDPATH += \
        ../src \
        ../include \

INCLUDEPATH += \
     ../include \
     ../src

OBJECTS_DIR = ../build
MOC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR
DESTDIR = ../lib

HEADERS += ../include/miil/hybrid_socket.h

SOURCES += ../src/hybrid_socket.cpp
