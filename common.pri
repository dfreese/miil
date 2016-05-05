TEMPLATE = lib
VERSION = 4.2.2
CONFIG -= qt
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic

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
