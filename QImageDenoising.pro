#-------------------------------------------------
#
# Project created by QtCreator 2014-06-28T19:16:51
#
#-------------------------------------------------

QT       += core

QT       += gui

TARGET = QImageDenoising
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    awgn.cpp \
    noise.cpp \
    median_filter.cpp \
    #nlm_filter.cpp

HEADERS += \
    awgn.h \
    noise.h \
    median_filter.h \
    #nlm_filter.h

IMAGE_FILES += \
    Lenna.png

win32 {
    DESTDIR_WIN = $${DESTDIR}
    DESTDIR_WIN ~= s,/,\\,g
    PWD_WIN = $${PWD}
    PWD_WIN ~= s,/,\\,g
    for(FILE, IMAGE_FILES){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${PWD_WIN}\\$${FILE} $${DESTDIR_WIN}$$escape_expand(\\n\\t))
    }
}
unix {
    for(FILE, IMAGE_FILES){
        QMAKE_POST_LINK += $$quote(cp $${PWD}/$${FILE} $${DESTDIR}$$escape_expand(\\n\\t))
    }
}
