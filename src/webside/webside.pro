######################################################################
# Automatically generated by qmake (2.01a) Tue Jul 13 15:38:09 2010
######################################################################

TEMPLATE = app
include(../../config.pri)
TARGET = sketch.wt
DEPENDPATH += . inc src
INCLUDEPATH += . inc

# Wt
INCLUDEPATH += /usr/local/include/Wt
LIBS += -lwtfcgi

# Common library
INCLUDEPATH += ../common/inc \
               ../common/common/inc \
               ../common/thread/inc \
               ../common/exception/inc \
               ../common/odbc/inc
LIBS += -lcommon

# DbSession (from service)
INCLUDEPATH += ../service/inc
DEPENDPATH += ../service/inc \
              ../service/src
HEADERS += DbSession.h \
           ChannelInternal.h \
           DataMarkInternal.h \
           UserInternal.h \
           DynamicCastFailure.h
SOURCES += DbSession.cpp \
           ChannelInternal.cpp \
           DataMarkInternal.cpp \
           UserInternal.cpp \
           DynamicCastFailure.cpp

# Input
HEADERS += inc/ChannelsWidget.h \
           inc/MarksWidget.h \
           inc/MarksModel.h \
           inc/OptionsModel.h \
           inc/OptionsWidget.h \
           inc/sketch.h
SOURCES += src/ChannelsWidget.cpp \
           src/main.cpp \
           src/MarksModel.cpp \
           src/MarksWidget.cpp \
           src/OptionsModel.cpp \
           src/OptionsWidget.cpp \
           src/sketch.cpp
