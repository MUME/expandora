######################################################################
# Automatically generated by qmake (1.07a) Wed Apr 13 17:57:43 2005
######################################################################

TEMPLATE = lib
INCLUDEPATH += . ../include/

CONFIG		+= qt opengl warn_on debug thread


# Input
HEADERS += Display.h MainWindow.h Terrain.h CachedRoom.h
SOURCES += Display.cpp MainWindow.cpp Terrain.cpp CachedRoom.cpp

LIBS += -L../lib -lCommon

DESTDIR = ../lib
