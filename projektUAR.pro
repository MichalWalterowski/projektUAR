QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = UARSimulator
TEMPLATE = app

SOURCES += main.cpp \
           dialogarx.cpp \
           mainwindow.cpp \
           UAR.cpp \
           qcustomplot.cpp \
           dialogarx.cpp

HEADERS  += mainwindow.h \
            UAR.h \
            dialogarx.h \
            qcustomplot.h \
            dialogarx.h

FORMS    += mainwindow.ui \
    dialogarx.ui
