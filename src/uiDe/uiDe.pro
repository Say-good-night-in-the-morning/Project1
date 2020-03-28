#-------------------------------------------------
#
# Project created by QtCreator 2020-02-18T19:00:28
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Cni
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

CONFIG += c++11

SOURCES += \
        adjust.cpp \
        decode.cpp \
        decodemethod.cpp \
        encode.cpp \
        error.cpp \
        main.cpp \
        mainwindow.cpp


HEADERS += \
        adjust.h \
        cvvariables.h \
        decode.h \
        encode.h \
        error.h \
        mainwindow.h \
        metho.h

FORMS += \
        adjust.ui \
        decode.ui \
        encode.ui \
        mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

INCLUDEPATH += D:/opencv/build/include

CONFIG(release, debug|release): LIBS += -LD:/opencv/build/x64/vc15/lib/ -lopencv_world420
else:CONFIG(debug, debug|release): LIBS += -LD:/opencv/build/x64/vc15/lib/ -lopencv_world420d

CONFIG(release, debug|release): LIBS += -LD:/opencv/build/x64/vc15/bin/ -lopencv_world420
else:CONFIG(debug, debug|release): LIBS += -LD:/opencv/build/x64/vc15/bin/ -lopencv_world420d
