# #####################################################################
# Automatically generated by qmake (2.01a) Mon May 10 12:33:04 2010
# #####################################################################
TEMPLATE = app
TARGET = 

QT += network

CONFIG += mobility
MOBILITY = location \
           bearer

DEPENDPATH += . \
              ./requests \
              ../gui/inc \
              ../gui/src
INCLUDEPATH += . \
               ./requests \
               ../gui/inc \
               ../common/inc

# Input
SOURCES += main.cpp \
           mainwindow.cpp \
           settingsdialog.cpp \
           ApplyChannelQuery.cpp \
           AddUserQuery.cpp \
           ../gui/src/SubscribeChannelQuery.cpp \
           ../gui/src/SubscribeChannelJSON.cpp \
           ../gui/src/ApplyMarkQuery.cpp \
           ../gui/src/LoginQuery.cpp

HEADERS += mainwindow.h \
           settingsdialog.h \
           tracker.h \
           ApplyChannelQuery.h \
           AddUserQuery.h \
           ../common/inc/defines.h \
           ../gui/inc/SubscribeChannelQuery.h \
           ../gui/inc/ApplyMarkQuery.h \
           ../gui/inc/LoginQuery.h

FORMS += formCreateChannel.ui

LIBS += -lqjson


