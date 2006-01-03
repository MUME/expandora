#ifndef DEFINES_H 
#define DEFINES_H 

#include <Qt>

#define MAX_ROOMS       30000		/* maximal amount of rooms */


#define MAX_STR_LEN     400
#define MAX_DATA_LEN 3072
#define MAX_LINES_DESC  20

//#define DIST_Z	2	/* the distance between 2 rooms */
#define BASE_Z  -12	/* the distance to the "camera" */
#define ROOM_SIZE 0.4f	/* the size of the rooms walls */

#define PROXY_BUFFER_SIZE 4096

#define NORTH           0         
#define EAST            1
#define SOUTH           2         
#define WEST            3
#define UP              4
#define DOWN            5

/* coordinate's cap */
#define MAX_X           32000
#define MIN_X           -32000


/* global flags */
extern int glredraw;		/* redraw is needed */
extern int glquit;			/* quiting ... */
extern int mud_emulation;          /* we are in emulation mode */

void toggle_renderer_reaction();
void notify_analyzer();


#endif

