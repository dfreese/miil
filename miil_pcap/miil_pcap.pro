include(../common.pri)
TARGET = ../lib/miil_pcap
LIBS += -lpcap
LIBS += -L../lib -lmiil_core
include(miil_pcap.pri)
