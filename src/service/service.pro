TEMPLATE = app
include(../../config.pri)
TARGET = fcgi_server
DEPENDPATH += . \
              inc \
              src \
              ../common/inc \
              ../common/src \
              ../common/common/inc \
              ../common/common/src \
              ../common/exception/inc \
              ../common/exception/src \
              ../common/odbc/inc \
              ../common/odbc/src \
              ../common/thread/inc \
              ../common/thread/src

INCLUDEPATH += \
               . \
               inc \
               ../common/inc \
               ../common/odbc/inc \
               ../common/common/inc \
               ../common/exception/inc \
               ../common/thread/inc \
               ../fcgiio/inc \
               ../json/inc

# Input
HEADERS += \
           inc/DynamicCastFailure.h \
           inc/engine.h \
           inc/server.h \
           inc/ChannelInternal.h \
           inc/DataMarkInternal.h \
           inc/UserInternal.h \
           inc/DbSession.h \
				   inc/UpdateThread.h \
           inc/QueryExecutor.h \
#				   ../json/inc/LoginRequestJSON.h \
#		  	   ../json/inc/LoginResponseJSON.h \
#				   ../json/inc/AddUserRequestJSON.h \
#		  	   ../json/inc/AddUserResponseJSON.h \
#				   ../json/inc/AddNewMarkRequestJSON.h \
#	     	   ../json/inc/AddNewMarkResponseJSON.h \
# 	 		   ../json/inc/SubscribeChannelResponseJSON.h \
#   		   ../json/inc/RSSFeedRequestJSON.h \
#           ../json/inc/RSSFeedJSON.h \
#					 ../json/inc/AddChannelRequestJSON.h \
#					 ../json/inc/AddChannelResponseJSON.h \


SOURCES += src/main.cpp \
           src/server.cpp \
           src/ChannelInternal.cpp \
           src/DataMarkInternal.cpp \
           src/DynamicCastFailure.cpp \
           src/UserInternal.cpp \
           src/DbSession.cpp \
           src/UpdateThread.cpp \
           src/QueryExecutor.cpp

LIBS +=  -lcommon -lfcgi -lwikigpsJson -lpq 
OBJECTS_DIR = .obj

QT -= gui
QT += sql
