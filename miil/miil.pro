TEMPLATE = lib
VERSION = 3.0.0
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic
LIBS += -L../lib -lmiil_core
LIBS += -L../lib -lmiil_ftdi
LIBS += -L../lib -lmiil_config
LIBS += -L../lib -lmiil_pcap
LIBS += -L../lib -lmiil_gui

DESTDIR = ../lib
