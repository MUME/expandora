//#include <qstring.h>
//#include "../SearchTreeNode.h"
//#include "../Room.h"
#include <stdio.h>
#include <set>

set<int> * baum = new set<int>;

int main(int argc, char **argv) {
  
  printf("sizeof(set<int>): %i\n", sizeof(baum));
//SearchTreeNode * test = new SearchTreeNode("baum");

/*
	char * string = new char[128];
	string[127] = 0;
	TinyList<char> l;
	for (char i = 127; i >= 0; i--) {
		string[i] = i+1;
		l.put(i, strdup(string));
		printf("put at %i: %s\n", i, string);
	}
	for (char i = 0; i < 128; i++) {
		printf("found at %i: %s\n", i, l.get(i));
	}	
	ParseEvent * event;
	ObjectRecycler<ParseEvent> mm;
	for(int i = 0; i< 10; i++) {
		event = new ParseEvent();
		printf("created event: %i\n", event);
		mm.deactivate(event);
		printf("deactivated event: %i\n", event);
	}
	printf("\n\nnext turn ...\n\n");
	for (int i = 0; i < 12; i++) {
		event = mm.activate();
		//event = event->mm.activate();
		printf("activated event: %i\n", event);
	}
	printf("ParseEvent: %i", sizeof(ParseEvent));
	printf("Property: %i", sizeof(Property));
	printf("QString: %i", sizeof(QString));
	*/
	return 0;
}
