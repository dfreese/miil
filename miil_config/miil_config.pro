include(../common.pri)
TARGET = ../lib/miil_config
LIBS += -ljsoncpp
LIBS += -L../lib -lmiil_core
include(miil_config.pri)
