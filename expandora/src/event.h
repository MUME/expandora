#ifndef C_EVENT_H
#define C_EVENT_H

#define E_ROOT '0' /* zero/root element */

#define E_NONE          0
#define E_CAUSE         1
#define E_RESULT        2


#define E_EMPTY         0

/* cause stacks */
#define C_MOVE 'M'
#define C_LOOK 'L'
#define C_SCOUT 'S'

/* result stacks */
#define R_ROOM 'R'
#define R_EXITS 'E'
#define R_BLIND 'B'	/* Entered in room while blinded, or in fog, or without light in dark  */
#define R_PROMPT 'P'
#define R_FAIL 'f'
#define R_DESC 'D'	/* room description */


extern struct event_types_type event_types[];


struct Tevent {
  char type;
  QByteArray data;		/* shall be enough for ANY roomdesc */	
  struct Tevent *next, *prev;
};

/* Cause Events top and bottom pointers
  new events are added to the bottom,
  and top ones are taken for analyzing */
extern struct Tevent *Ctop, *Cbottom; /* events that cause movements */
extern struct Tevent *Rtop, *Rbottom; /* response on them */

extern struct Tevent *pre_Cstack;
extern struct Tevent *pre_Rstack;

extern QMutex stacks_mutex;


void preRAdd(char type, QByteArray data);
void preCAdd(char type, QByteArray data);


void Rremove(); /* remove an items from Result stack */
void Cremove(); /* remove an items from Result stack */
void addtostack(struct Tevent *stack, char type, QByteArray data);
void printstacks();

char get_cause_type_by_line(char *line);
char get_result_type_by_line(char *line);


void clear_events_stacks();

#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif
