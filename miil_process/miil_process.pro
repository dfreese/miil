include(../common.pri)
TARGET = ../lib/miil_process
LIBS += -L../lib -lmiil_core
LIBS += -L../lib -lmiil_config
LIBS += -lpthread

CXX = g++-4.7
QMAKE_CXX = g++-4.7
QMAKE_CC = gcc-4.7

include(miil_process.pri)
