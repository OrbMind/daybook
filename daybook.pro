#-------------------------------------------------
#
# Project created by QtCreator 2016-01-06T15:22:32
#
#-------------------------------------------------

QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = daybook
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialogjob.cpp \
    dialogjobedit.cpp \
    dialogusers.cpp \
    dialoguseredit.cpp \
    dialogdirection.cpp \
    dialogabout.cpp \
    dialogentersoft.cpp \
    maindef.cpp

HEADERS  += mainwindow.h \
    dialogjob.h \
    dialogjobedit.h \
    dialogusers.h \
    dialoguseredit.h \
    dialogdirection.h \
    maindef.h \
    dialogabout.h \
    dialogentersoft.h

FORMS    += mainwindow.ui \
    dialogjob.ui \
    dialogjobedit.ui \
    dialogusers.ui \
    dialoguseredit.ui \
    dialogdirection.ui \
    dialogabout.ui \
    dialogentersoft.ui

RESOURCES += \
    images.qrc
