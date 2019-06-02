RCC_DIR = tmp
OBJECTS_DIR = tmp
UI_DIR = tmp
MOC_DIR = tmp
DESTDIR = exec
DEFINES += QT5 _TTY_WIN_
QT += widgets sql network serialport websockets

DEPENDPATH += src inc
INCLUDEPATH += . inc
TEMPLATE = app
TARGET = ParamSettings

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/uart.cpp \
    src/lcmslave.cpp

HEADERS += \
    inc/mainwindow.h \
    inc/uart.h \
    inc/usertype.h \
    inc/lcmslave.h

FORMS += \
    ui/mainwindow.ui


