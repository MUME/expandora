
#define TIMESTAMP_FORMAT "dd.MM.yyyy - hh:mm:ss"
#define TIMESTAMP_LEN    21

struct Troom {
  unsigned int id; 		/* identifier */
  char *name; 			/* POINTER to the room name */
  char *note; 			/* note, if needed, additional info etc */
  char *desc;			/* descrition */
  char sector;                 /* terrain marker */ 		
                                /* _no need to free this one_ */
  unsigned int exits[6];	/* 0 if no connection, id if there is any */ 
  char *doors[6];		/* if the door is secret */

  char timestamp[ TIMESTAMP_LEN + 1 ];
  
  int x, y, z;		/* coordinates on our map */
  struct Troom *prev, *next;    /* list organisation */
};

struct room_flag_data {
  char *name;
  char *xml_name;
  unsigned int flag;
};


struct event_types_type {
  QByteArray    name;
  char          type;
  char          group;
};

struct engine_flags_type {
  char done;                    /* finish */       
  char addingroom;              /* adding room right now */
  char resync;                  /* do full resync */
  char mapping;                 /* mapping is On/OFF */
  char gettingfirstroom;        /* getting the very first room in base */
  
  
  char redraw;  
  
  QByteArray last_roomname;
  QByteArray last_desc;
  QByteArray last_exits;
  char last_terrain;
};

extern struct engine_flags_type engine_flags;

#ifdef DMALLOC
#include <dmalloc.h>
#endif
