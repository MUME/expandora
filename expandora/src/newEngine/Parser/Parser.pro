######################################################################
# Automatically generated by qmake (1.07a) Fri Apr 15 13:19:32 2005
######################################################################

TEMPLATE = lib
INCLUDEPATH += . ../include/
DESTDIR = ../lib/
LIBS += -L../lib -lCommon
OBJECTS_DIR	= obj
MOC_DIR		= moc
CONFIG += debug thread qt opengl warn_on
win32 {
	CONFIG	+= console
}

# Input
HEADERS += Parser.h Path.h Approved.h Experimenting.h Syncing.h PathParameters.h RoomSignalHandler.h

SOURCES += Parser.cpp Path.cpp Approved.cpp Experimenting.cpp Syncing.cpp PathParameters.cpp RoomSignalHandler.cpp
debug {
  #DEFINES += DMALLOC
  #LIBS += -ldmallocthcxx
}
