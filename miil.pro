TEMPLATE = lib
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic
LIBS += -lftd2xx

DEPENDPATH += \
        src \
        include \

INCLUDEPATH += \
     include \
     src

OBJECTS_DIR = $$OUT_PWD/build
DESTDIR = $$OUT_PWD/lib

HEADERS += \
    include/miil/ethernet.h \
    include/miil/raw_socket.h \
    include/miil/standard_socket.h \
    include/miil/usbport.h \
    include/miil/usbport1.h \
    include/miil/usbport2.h \
    include/miil/util.h

SOURCES += \
    src/ethernet.cpp \
    src/raw_socket.cpp \
    src/standard_socket.cpp \
    src/usbport.cpp \
    src/usbport1.cpp \
    src/usbport2.cpp \
    src/util.cpp
