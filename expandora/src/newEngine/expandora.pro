
TEMPLATE	= app
OBJECTS_DIR	= obj
MOC_DIR		= moc

CONFIG		+= qt opengl warn_on debug thread
win32 {
	CONFIG	+= console
}
macx {
	HEADERS += cmath
}

LEXSOURCES	+=mud.lex
LEXSOURCES	+=player.lex

HEADERS         +=MainWindow.h
HEADERS         +=Configuration.h
HEADERS         +=Proxy.h
HEADERS         +=Display.h
HEADERS         +=XmlStorage.h
HEADERS         +=ListCycler.h
HEADERS         +=SimpleString.h
HEADERS		+=BaseEvent.h
HEADERS		+=CharacterTable.h
HEADERS		+=Coordinate.h
HEADERS		+=IntermediateNode.h
HEADERS		+=LexDefs.h
HEADERS		+=Lexer.h
HEADERS		+=Map.h
HEADERS		+=ObjectRecycler.h
HEADERS		+=ParseEvent.h
HEADERS		+=Parser.h
HEADERS		+=Path.h
HEADERS		+=Property.h
HEADERS		+=Room.h
HEADERS		+=RoomAdmin.h
HEADERS		+=RoomCollection.h
HEADERS		+=RoomSearchNode.h
HEADERS		+=SearchTreeNode.h
HEADERS		+=Terrain.h
HEADERS		+=TinyList.h

SOURCES         +=Controller.cpp
SOURCES         +=MainWindow.cpp
SOURCES         +=Configuration.cpp
SOURCES         +=Proxy.cpp
SOURCES         +=Display.cpp
SOURCES         +=XmlStorage.cpp
SOURCES         +=SimpleString.cpp
SOURCES         +=TelnetFilter.cpp
SOURCES		+=BaseEvent.cpp
SOURCES		+=Terrain.cpp
SOURCES		+=CharacterTable.cpp
SOURCES		+=Coordinate.cpp
SOURCES		+=IntermediateNode.cpp
SOURCES		+=Lexer.cpp
SOURCES		+=Map.cpp
SOURCES		+=ParseEvent.cpp
SOURCES		+=Parser.cpp
SOURCES		+=Path.cpp
SOURCES		+=Property.cpp
SOURCES		+=Room.cpp
SOURCES		+=RoomAdmin.cpp
SOURCES		+=RoomCollection.cpp
SOURCES		+=SearchTreeNode.cpp

TARGET		= ../expandora

!mac:unix:LIBS		+= -lm -lqt-mt



