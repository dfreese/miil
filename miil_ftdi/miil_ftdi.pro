include(../common.pri)
TARGET = ../lib/miil_ftdi
LIBS += -lftd2xx
LIBS += -L../lib -lmiil_core
include(miil_ftdi.pri)
