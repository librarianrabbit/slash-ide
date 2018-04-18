#-------------------------------------------------
#
# Project created by QtCreator 2013-05-29T21:03:48
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SlashIDE
TEMPLATE = app

DEFINES += QSCINTILLA_DLL

SOURCES += main.cpp\
        mainwindow.cpp \
    luaeditor.cpp \
    slashapi.cpp \
    slashapifunction.cpp \
    slashapinamespace.cpp \
    slashapienum.cpp \
    slashapivar.cpp \
    lualexer.cpp \
    slashapiobject.cpp \
    utils.cpp \
    finder.cpp \
    gotodialog.cpp \
    encloseselectiondialog.cpp \
    modifylinesdialog.cpp \
    qclickablelabel.cpp

HEADERS  += mainwindow.h \
    luaeditor.h \
    slashapi.h \
    slashapifunction.h \
    slashapinamespace.h \
    slashapienum.h \
    slashapivar.h \
    lualexer.h \
    slashapiobject.h \
    utils.h \
    finder.h \
    gotodialog.h \
    encloseselectiondialog.h \
    modifylinesdialog.h \
    utils.hxx \
    qclickablelabel.h

FORMS    += mainwindow.ui \
    finder.ui \
    gotodialog.ui \
    encloseselectiondialog.ui \
    modifylinesdialog.ui

RESOURCES += \
    resources.qrc

win32:RC_FILE = win32rc.rc
