TEMPLATE = lib
CONFIG += staticlib dll
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic

DEPENDPATH += . \
        src \
        include

INCLUDEPATH += . \
     include \
     src

OBJECTS_DIR = $$OUT_PWD/build
DESTDIR = $$OUT_PWD/lib

HEADERS += \
    include/ethernet.h \
    include/raw_socket.h \
    include/standard_socket.h

SOURCES += \
    src/ethernet.cpp \
    src/raw_socket.cpp \
    src/standard_socket.cpp
