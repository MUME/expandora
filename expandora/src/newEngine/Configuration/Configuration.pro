
######################################################################
# Automatically generated by qmake (1.07a) Wed Apr 13 15:01:09 2005
######################################################################

TEMPLATE = lib
INCLUDEPATH += . ../include/
OBJECTS_DIR	= obj
MOC_DIR		= moc
CONFIG += debug thread qt opengl warn_on
win32 {
	CONFIG	+= console
}

# Input
HEADERS += Configuration.h
SOURCES += Configuration.cpp

DESTDIR = ../../../lib/
LIBS += -L../../../lib -lCommon
