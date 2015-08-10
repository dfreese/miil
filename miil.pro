TEMPLATE = lib
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic
LIBS += -lftd2xx -lqcustomplot

DEPENDPATH += \
        src \
        include \

INCLUDEPATH += \
     include \
     src

OBJECTS_DIR = $$OUT_PWD/build
MOC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR
DESTDIR = $$OUT_PWD/lib

HEADERS += \
    include/miil/comm.h \
    include/miil/ethernet.h \
    include/miil/file_utils.h \
    include/miil/hvcontroller.h \
    include/miil/hvcontrolwidget.h \
    include/miil/instekpowersupply.h \
    include/miil/log.h \
    include/miil/raw_socket.h \
    include/miil/standard_socket.h \
    include/miil/temprhmonitor.h \
    include/miil/TimeGraph.h \
    include/miil/usbport.h \
    include/miil/usbport1.h \
    include/miil/usbport2.h \
    include/miil/util.h

SOURCES += \
    src/comm.cpp \
    src/ethernet.cpp \
    src/file_utils.cpp \
    src/hvcontroller.cpp \
    src/hvcontrolwidget.cpp \
    src/instekpowersupply.cpp \
    src/log.cpp \
    src/raw_socket.cpp \
    src/standard_socket.cpp \
    src/temprhmonitor.cpp \
    src/TimeGraph.cpp \
    src/usbport.cpp \
    src/usbport1.cpp \
    src/usbport2.cpp \
    src/util.cpp

FORMS    += \
    ui/hvcontrolwidget.ui
