TEMPLATE = app
include(../../config.pri)
TARGET = fcgi_server

INSTALLS += target conf_lighttpd odbc_configs postgre_conf database user_managment libriak

target.path = /opt/geo2tag/

libriak.files = ../../3rdparty/libriak/.libs/libriak_client.so.1.0.0 ../../3rdparty/libriak/.libs/libriak_client.so.1 ../../3rdparty/libriak/.libs/libriak_client.so
libriak.path = /usr/lib/

conf_lighttpd.files = wikigps-service.conf
conf_lighttpd.path = /etc/lighttpd/conf-enabled

postgre_conf.files = ../../conf/pg_hba.conf
postgre_conf.path = /opt/geo2tag/conf/

db_content.files = base.sql
db_content.path = /opt/geo2tag/

odbc_configs.files = ../../scripts/odbc.ini ../../scripts/odbcinst.ini
odbc_configs.path = /opt/geo2tag/

database.files=../../scripts/*
database.path=/opt/geo2tag/db/

user_managment.files=../user_managment/*
user_managment.path=/var/www/

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
               ../inc/ \
               ../common/inc \
               ../common/odbc/inc \
               ../common/common/inc \
               ../common/exception/inc \
               ../common/thread/inc \
               ../fcgiio/inc \
               ../json/inc \


# Input
HEADERS += \
           inc/DynamicCastFailure.h \
           inc/server.h \
           inc/ChannelInternal.h \
           inc/DataMarkInternal.h \
           inc/UserInternal.h \
           inc/QueryExecutor.h \
    inc/SessionInternal.h \
    inc/Geo2tagDatabase.h \
    inc/DbObjectsCollection.h \
    inc/MetaCache.h \
    inc/RiakInteractor.h


SOURCES += src/main.cpp \
           src/server.cpp \
           src/ChannelInternal.cpp \
           src/DataMarkInternal.cpp \
           src/DynamicCastFailure.cpp \
           src/UserInternal.cpp \
           src/DbObjectsCollection.cpp \
           src/QueryExecutor.cpp \
    src/SessionInternal.cpp \
    src/Geo2tagDatabase.cpp \
    processors/ProcessorsCore.cpp \
    processors/ProcessorsFilters.cpp \
    src/MetaCache.cpp \
    src/RiakInteractor.cpp

INCLUDEPATH += ../../3rdparty/libriak/riak_client
INCLUDEPATH += ../../3rdparty/libriak

LIBS +=  -lcommon -lfcgi -lwikigpsJson -lpq -lriak_client
OBJECTS_DIR = .obj

QMAKE_LFLAGS += -L../lib -L/usr/lib/ -L../../3rdparty/libriak/lib

QT -= gui
QT += sql

OTHER_FILES += \
    ../../scripts/base.sql \
    ../../conf/pg_hba.conf
