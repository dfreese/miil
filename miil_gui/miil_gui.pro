include(../common.pri)
TARGET = ../lib/miil_gui
CONFIG += qt
LIBS += -lqcustomplot
LIBS += -L../lib -lmiil_core
include(miil_gui.pri)
