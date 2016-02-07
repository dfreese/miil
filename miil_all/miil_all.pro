include(../common.pri)
CONFIG += qt
LIBS += -lpcap -lftd2xx -ljsoncpp -lqcustomplot
TARGET = ../lib/miil
include(../miil_core/miil_core.pri)
include(../miil_ftdi/miil_ftdi.pri)
include(../miil_config/miil_config.pri)
include(../miil_pcap/miil_pcap.pri)
include(../miil_gui/miil_gui.pri)
