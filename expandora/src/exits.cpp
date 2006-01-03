#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <QMutex>


#include "Map.h"
#include "configurator.h"
#include "defines.h"
#include "exits.h"
#include "dispatch.h"
#include "stacks.h"
#include "event.h"
#include "forwarder.h"
#include "utils.h"
#include "engine.h"


void do_exits(const char *exits_line)
{
    int exits[6];
    unsigned int i;
    CRoom *r;

    parse_exits(exits_line, exits);

    if (Engine.isAddingroom()) {
	   r = Engine.addedroom;

        
       print_debug(DEBUG_ANALYZER /*& DEBUG_TOUSER*/, 
                    "Exits: Adding exits information to the new room.");

       for (i = 0; i <= 5; i++) {
            if (r->exits[i] > 0) {
                if (exits[i] == 0) {	/* oneway case */
                    Map.oneway_room_id = r->exits[i];
                    r->exits[i] = 0;
                }
                
                if (exits[i] == E_CLOSEDDOOR) 
                    r->doors[i] = strdup("exit");
                
                
                continue;
            }
            if (exits[i] == E_CLOSEDDOOR) {
                r->doors[i] = strdup("exit");
            }
            
            if (exits[i] == E_PORTAL) {
                send_to_user("--[AUDA: attention PORTAL was in room as you entered it. Fix existing exits if needed\r\n");
            }
            
            if ((exits[i] > 0) && (exits[i] != E_PORTAL))
                r->exits[i] = EXIT_UNDEFINED;
            }

        stacker.put(Engine.addedroom);
            
        return;
    }

    if (conf.get_exits_check() ) 
      print_debug(DEBUG_ANALYZER, "Autochecking exits");
      
    for (i = 0; i < stacker.amount(); i++) {
      r = stacker.get(i);
      if (conf.get_exits_check() ) {
        if (compare_exits(r, exits)) {
          stacker.put(r);
          print_debug(DEBUG_ANALYZER, "adding match");
        }
      } else {
        print_debug(DEBUG_ANALYZER, "adding all matches without check. its off");
        stacker.put(r);
      }
    }
    
}

int compare_exits(CRoom *p, int exits[])
{
    int counter;
    int i;

    counter = 0;
    /* for each door there are following situations : */
    /* 0) closed door and secret exit in base -> skip this one */
    /* 1) open exit and normal exit in base -> skip! */
    /* a) normal exit there and exit in base (secret or not doesnt matter) */
    /* b) no exit and secret exit in base -> fine */

    print_debug(DEBUG_ANALYZER, "compare_exits called.");
    for (i = 0; i <= 5; i++) {
        if ((exits[i] == 3) && (p->exits[i] > 0) && (p->doors[i] != NULL))
            if (strcmp("exit", p->doors[i]) != 0)
                break;		/* situation 0) */

            if (exits[i] == 4) {
                counter++;		/* portal - we ignore this exit and count as possible */
	    	continue;
            }

            if ((exits[i] == 2) && (p->exits[i] > 0) && (p->doors[i] == NULL))
	    	break;		/* situation 1) */


            if ((exits[i] > 0) && (p->exits[i] > 0)) {
	    	counter++;
	    	continue;
            }

            if ((exits[i] == 0) && (p->exits[i] > 0) && (p->doors[i] != NULL))
	    	if (strcmp("exit", p->doors[i]) != 0) {
                    counter++;
                    continue;
	    	}
            
            if ((exits[i] == 0) && (p->exits[i] == 0)) {
                counter++;
                continue;
            }
    }

    if (counter == 6) {		/* only if all 6 exits are the same we can say it probably the same room */
        print_debug(DEBUG_ANALYZER, "Compare exits result - positive.");
        return 1;
    }
    print_debug(DEBUG_ANALYZER, "Compare exits result - negative.");
    return 0;
}


void parse_exits(const char *p, int exits[])
{
    const char *exit_patterns[] = {
      "no",
      "ea",
      "so",
      "we",
      "up",
      "do"
    };
    
    
    int len;
    int i;
    int dir;
  
    /* NOrth, EAst, SOuth, WEst, UP, DOwn */
    /* output format : */
    /* 0 - no exit there, 1 - normal exit, 2 - open door, 3 - closed door, 4 - portal */

    print_debug(DEBUG_ANALYZER, "Parsing exits line");
    if (p == NULL) {
        printf("parse_exits: ERROR - NULL EVENT GIVEN!\r\n");
    }

    exits[0] = E_NOEXIT;
    exits[1] = E_NOEXIT;
    exits[2] = E_NOEXIT;
    exits[3] = E_NOEXIT;
    exits[4] = E_NOEXIT;
    exits[5] = E_NOEXIT;


    len = strlen(p);
    for (i = 0; i <= len; i++) 
      for (dir = 0; dir <= 5; dir++) 
        if ((p[i] == exit_patterns[dir][0]) && 
            (p[i + 1] == exit_patterns[dir][1])) 
        {
            if (i == 0) {
                exits[dir] = E_NORMAL;
                continue;
            }
            if (p[i - 1] == '(') {
                exits[dir] = E_OPENDOOR;
                continue;
            }
            if (p[i - 1] == '[') {
                exits[dir] = E_CLOSEDDOOR;
                continue;
            }
            if (p[i - 1] == '<') {
                exits[dir] = E_PORTAL;
                continue;
            }
            exits[dir] = E_NORMAL;
            continue;
          }
      
    
}
