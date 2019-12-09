#-------------------------------------------------
#
# Project created by QtCreator 2019-10-28T09:17:38
#
#-------------------------------------------------

QT       += core gui
QT       += datavisualization
QT       += widgets # File Dialog
#QT       += qwt

greaterThan(QT_MAJOR_VERSION, 4     ): QT += widgets

TARGET = SignalAnalyzer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17
CONFIG += qwt

# Include Path
INCLUDEPATH += /usr/include/qwt
INCLUDEPATH += processing

# Libs and Flags
QMAKE_CXXFLAGS += -fopenmp -O3 -mavx -mfma -march=native
LIBS += -fopenmp -lfftw3f -lqwt-qt5 # -lfftw3f

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        psd.cpp \
        scdf0.cpp \
        td.cpp \
        scd.cpp \
        spectogram.cpp \
        signalmanager.cpp \
        processing/psdwelch.cpp \
        processing/scdf0welch.cpp \
        processing/scd_fam_r_single.cpp \
        signaltransform.cpp \
        waterfall.cpp \
        iq.cpp \
        processing/windowfunctions.cpp

HEADERS += \
        mainwindow.h \
        mytypes.h \
        psd.h \
        scdf0.h \
        td.h \
        scd.h \
        processing/scd_fam_r_single.h \
        spectogram.h \
        signalmanager.h \
        processing/psdwelch.h \
        signaltransform.h \
        waterfall.h \
        iq.h \
        processing/scdf0welch.h \
        processing/windowfunctions.h


FORMS += \
        ui/mainwindow.ui \
        ui/psd.ui \
        ui/scdf0.ui \
        ui/td.ui \
        ui/scd.ui \
        ui/signaltransform.ui \
        ui/waterfall.ui \
        ui/iq.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
