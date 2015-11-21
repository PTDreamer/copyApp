QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = copyApp.bin
win32 {
TARGET = copyApp
}
macx {
TARGET = copyApp
}

TEMPLATE = app


SOURCES += main.cpp\
        copyapp.cpp

HEADERS  += copyapp.h

FORMS    += copyapp.ui

#Windows icon and admin right
win32 {
    RC_FILE = copyApp.rc
    LIBS *= Shell32.lib
    CONFIG -= embed_manifest_exe
}
