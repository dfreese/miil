TEMPLATE = lib
VERSION = 3.0.0
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic
LIBS += -lqcustomplot
LIBS += -L../lib -lmiil_core

DEPENDPATH += \
        ../src \
        ../include \

INCLUDEPATH += \
     ../include \
     ../src \

OBJECTS_DIR = ../build
MOC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR
DESTDIR = ../lib

HEADERS += \
    ../include/miil/hvcontrolwidget.h \
    ../include/miil/TimeGraph.h

SOURCES += \
    ../src/hvcontrolwidget.cpp \
    ../src/TimeGraph.cpp

FORMS    += \
    ../ui/hvcontrolwidget.ui
