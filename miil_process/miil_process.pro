include(../common.pri)
TARGET = ../lib/miil_process
LIBS += -L../lib -lmiil_core
LIBS += -L../lib -lmiil_config
LIBS += -lpthread

include(miil_process.pri)
