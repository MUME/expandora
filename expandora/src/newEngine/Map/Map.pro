######################################################################
# Automatically generated by qmake (1.07a) Fri Apr 15 12:51:01 2005
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
HEADERS += CharacterTable.h \
           IntermediateNode.h \
           Map.h \
           MapFrontend.h \
           RoomSearchNode.h \
	   RoomCollection.h \
           SearchTreeNode.h
SOURCES += CharacterTable.cpp \
           IntermediateNode.cpp \
           RoomCollection.cpp \
           Map.cpp \
           MapFrontend.cpp \
           SearchTreeNode.cpp

debug {
  #DEFINES += DMALLOC
  #LIBS += -ldmallocthcxx
}
