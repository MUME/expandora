######################################################################
# Automatically generated by qmake (1.07a) Thu Apr 14 21:24:30 2005
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
HEADERS += Lexer.h
SOURCES += Lexer.cpp

LEXSOURCES += mud.lex player.lex

DESTDIR = ../../../lib/
LIBS += -L../../../lib -lCommon
