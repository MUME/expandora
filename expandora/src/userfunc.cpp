#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qmutex.h>

#include "rooms.h"
#include "defines.h"
#include "struct.h"
#include "dispatch.h"
#include "stacks.h"
#include "forwarder.h"
#include "utils.h"
#include "userfunc.h"
#include "xml2.h"
#include "event.h"
#include "engine.h"
#include "tree.h"
#include "renderer.h"
#include "exits.h"


/* ================= ENCHANCED USER FUNCTIONS VERSIONS =============== */
#define USERCMD_FLAG_SYNC       (1 << 0)       /* sync is required */
#define USERCMD_FLAG_REDRAW     (1 << 1)       /* redraw after executing */

#define USER_CONF_MAP         1
#define USER_CONF_BRIEF       2
#define USER_CONF_EXITS       3
#define USER_CONF_DESCS       4
#define USER_CONF_TERRAIN     5

#define USER_MOVE_LOOK        6
#define USER_MOVE_EXAMINE     7

#define USER_CONF_AUTOMERGE   8
#define USER_CONF_ANGRYLINKER 9


#define userfunc_print_debug print_debug(DEBUG_USERFUNC, "called function cmd %i, subcmd %i", cmd, subcmd);

#define GET_INT_ARGUMENT(arg, value) \
  if (!is_integer(arg)) {               \
    send_to_user("--[ argument %s is not an integer as its supposed to be.\r\n", arg); \
    send_to_user(last_prompt); \
    return USER_PARSE_DONE;             \
  }                                             \
  value = atoi(arg);
  

#define MISSING_ARGUMENTS \
  { \
    send_to_user("--[Pandora: Missing arguments.\n");      \
    send_to_user(last_prompt);                          \
    return USER_PARSE_DONE;                             \
  }
  
  
#define PARSE_DIR_ARGUMENT(dir, arg) \
    dir = parse_dir(arg);               \
    if (dir == -1) {                            \
      send_to_user("--[ %s is not a dirrection.\r\n", arg);     \
      send_to_user(last_prompt); \
      return USER_PARSE_DONE;             \
    }                                             

#define CHECK_SYNC \
        if (stacker.amount != 1) {      \
          send_to_user("--[Pandora: Current position is undefined(out of sync).\n");       \
          send_to_user(last_prompt);    \
          return USER_PARSE_DONE;       \
        }

    
void display_debug_settings();
  
USERCMD(usercmd_mhelp);
USERCMD(usercmd_config);

USERCMD(usercmd_msave);
USERCMD(usercmd_mload);
USERCMD(usercmd_mreset);
USERCMD(usercmd_mstat);
USERCMD(usercmd_minfo);
USERCMD(usercmd_move);
USERCMD(usercmd_mmerge);

USERCMD(usercmd_mnewmap);


#define USER_DEC_X      1
#define USER_DEC_Y      2
#define USER_DEC_Z      3
#define USER_INC_X      4
#define USER_INC_Y      5
#define USER_INC_Z      6
USERCMD(usercmd_mdec);

USERCMD(usercmd_mcoord);


#define USER_DOOR_NORMAL        0
#define USER_DOOR_EXIT          1
USERCMD(usercmd_mdoor);

USERCMD(usercmd_mmark);

USERCMD(usercmd_mlink);
USERCMD(usercmd_mdetach);

USERCMD(usercmd_mgoto);

USERCMD(usercmd_maction);

USERCMD(usercmd_mdebug);

USERCMD(usercmd_mdelete);

USERCMD(usercmd_mrefresh);

USERCMD(usercmd_mtreestats);

USERCMD(usercmd_mquit);

USERCMD(usercmd_maddroom);

USERCMD(usercmd_mevent);


const struct user_command_type user_commands[] = {
  {"maddroom",     usercmd_maddroom,          0,                        USERCMD_FLAG_REDRAW,   
    "Add current room to database [READ HELP]",
   "    Usage: maddroom \r\n"
   "    Examples: maddroom \r\n\r\n"
   "    Use this command for special cases (like rivendell entrances) when its impossible\r\n"
   "add/map the connection and following room in normal way(with mmap on and movement).\r\n"
   "This command takes the last seen roomname, description (optional), exits and terrain data\r\n"
   "of the last visited room and adds a room with coordinates 100, 100, 100.\r\n"},
  {"mhelp",     usercmd_mhelp,          0,                              0,   
    "Brief help files or commands overview.",
   "    Usage: mhelp [command name]\r\n"
   "    Examples: mhelp / mhelp mmap\r\n\r\n"
   "    This command gives a brief overview of all available commands when no arguments\r\n"
   "are given. Or you can see more detailed data for any given command.\r\n"},
  {"mmap",     usercmd_config,         USER_CONF_MAP,   USERCMD_FLAG_SYNC,   
    "Turn mapping mode on/off, if possible.",
   "    Usage: mmap [boolean]\r\n"
   "    Examples: mmap / mmap on / mmap false\r\n\r\n"
   "    This command turns mapping mode on/off. To turn the mapping mode on the analyzer needs\r\n"
   "to be 100% synced.\r\n"
   "    This will will automatically send commands brief off, spam on and prompt all to the game.\r\n"},
  {"mbrief",   usercmd_config,         USER_CONF_BRIEF,  0,   
    "Turn mappers built in brief mode on/off.",
   "    Usage: mbrief [boolean]\r\n"
   "    Examples: mbrief / mbrief on / mbrief true / mbrief 1 / mbrief yes\r\n\r\n"
   "    This command turns internal mappers brief mode on/off. When its on\r\n"
   "mapper will not show room descriptions to the user if any are recognised.\r\n"},
  {"mcheckexits",   usercmd_config,    USER_CONF_EXITS,   0,   
    "Turn exits analyzer on/off.",
   "    Usage: mcheckexits [boolean]\r\n"
   "    Examples: mcheckexits / mcheckexits on / mcheckexits true / mcheckexits 1 /mcheckexits yes\r\n\r\n"
   "    This command turns on the exits analyzing. Exits analyzer requires a very full\r\n"
   "map.\r\n" },
  {"mautomerge",   usercmd_config,   USER_CONF_AUTOMERGE,     0,   
    "Turn descriptions analyzer in mapping mode on/off.",
   "    Usage: mautomerge [boolean]\r\n"
   "    Examples: mautomerge / mautomerge on / mautomerge true / mautomerge 1 / mautomerge yes\r\n\r\n"
   "    This command turns the descriptions analyzer and automatic merging on/off. Turn it off in\r\n"
   "badly created zone and link rooms manually.\r\n"},
  {"mangrylinker",   usercmd_config,   USER_CONF_ANGRYLINKER,     0,   
    "Turn the AngryLinker on/off.",
   "    Usage: mangrylinker [boolean]\r\n"
   "    Examples: mangrylinker / mangrylinker on / mangrylinker true\r\n\r\n"
   "    AngryLinker attempts to link the surrounding rooms based on its coordinates.\r\n"
   "Be VERY careful using this - you can easilly get mistakenly linked rooms. Though this is a\r\n"
   "very helpful option for 10x10 zones. \r\n"},
  {"mcheckterrain",   usercmd_config,   USER_CONF_TERRAIN,     0,   
    "Turn terrain analyzer on/off.",
   "    Usage: mcheckterrain [boolean]\r\n"
   "    Examples: mcheckterrain / mcheckterrain on / mcheckterrain no / mcheckterrain 1\r\n\r\n"
   "    This command triggers the terrain analyzer (apply_prompt). \r\n"},
  {"msave",         usercmd_msave,         0,           0,   
    "Save/Save as current database.",
   "    Usage: msave [filename]\r\n"
   "    Examples: msave / msave warpmap.xml\r\n\r\n"
   "    This command saves the database stored in memory in an xml file. Without arguments\r\n"
   "it saves to the default file.\r\n"},
  {"mload",         usercmd_mload,         0,           USERCMD_FLAG_REDRAW,   
    "Load file/Reload the database from disk.",
   "    Usage: mload [filename]\r\n"
   "    Examples: mload / mload warpmap.xml\r\n\r\n"
   "    Without arguments this command reloads already opened database. As argument you can specify some\r\n"
   "other database file to load.\r\n"},
  {"mreset",         usercmd_mreset,         0,           USERCMD_FLAG_REDRAW,   
    "Reset mappers state stacks.",
   "    Usage: mreset\r\n"
   "    Examples: mreset\r\n\r\n"
   "    This command resets all stacks - current possible positions, cause and result stacks.\r\n"
   "This is very useful when something went wrong and some value stayed in movements stack.\r\n"
   "For example - you forgot to add some movement failure pattern to the config file. When this \r\n"
   "movement failure case will show up you will have to reset the stacks and resync manualy.\r\n"},
  {"mstat",         usercmd_mstat,         0,           0,   
    "Display settings and mappers state stacks.",
   "    Usage: mstat\r\n"
   "    Examples: mstat\r\n\r\n"
   "    This command displays settings, stacks and possible current position room id's\r\n"},
  {"minfo",         usercmd_minfo,         0,           0,   
    "Display current rooms data (or by given id).",
   "    Usage: minfo [id]\r\n"
   "    Examples: minfo / minfo 120\r\n\r\n"
   "    This command displays everything know about current room. Roomname, id, flags,\r\n"
   "room description, exits, connections and last update date.\r\n"},
  {"north",         usercmd_move,         NORTH,          0,   NULL, NULL},
  {"east",          usercmd_move,         EAST,           0,   NULL, NULL},
  {"south",         usercmd_move,         SOUTH,          0,   NULL, NULL},
  {"west",          usercmd_move,         WEST,           0,   NULL, NULL},
  {"up",            usercmd_move,         UP,             0,   NULL, NULL},
  {"down",          usercmd_move,         DOWN,           0,   NULL, NULL},
  {"look",          usercmd_move,         USER_MOVE_LOOK, 0,   NULL, NULL},
  {"l",             usercmd_move,         USER_MOVE_LOOK, 0,   NULL, NULL},
  {"look",          usercmd_move,         USER_MOVE_LOOK, 0,   NULL, NULL},
  {"examine",       usercmd_move,         USER_MOVE_LOOK, 0,   NULL, NULL},
  {"exa",           usercmd_move,         USER_MOVE_LOOK, 0,   NULL, NULL},
  {"exam",          usercmd_move,         USER_MOVE_LOOK, 0,   NULL, NULL},
  {"mmerge",        usercmd_mmerge,       0,    USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Merge twin rooms - manual launch.",
   "    Usage: mmerge [id] [force]\r\n"
   "    Examples: mmerge / mmerge 120 / mmerge 120 force\r\n\r\n"
   "Without arguments this command will try to merge last added room with either\r\n"
   "found room or given room. Force argument disables the roomname and desc checks.\r\n"},
   {"mnewmap",        usercmd_mnewmap,       0,           0,   
    "Start new map (loads current room as base room).",
   "    Usage: mnewmap\r\n"
   "    Examples: mnewmap\r\n\r\n"
   "Gets current room as base room for new map via sending examine command to the game.\r\n"},
  {"mdecx",             usercmd_mdec,        USER_DEC_X,   USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Decrease the X coordinate.",
   "    Usage: mdecx [integer]\r\n"
   "    Examples: mdecx / mdecx 2\r\n\r\n"
   "mdecx substracts 1 from the X coordinate. If you give an integer argument X, X will be substracted.\r\n"},
  {"mincx",             usercmd_mdec,        USER_INC_X,    USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Increase the X coordinate.",
   "    Usage: mincx [integer]\r\n"
   "    Examples: mincx / mincx 2\r\n\r\n"
   "mdecx adds 1 to the X coordinate. If you give an integer argument X, X will be added.\r\n"},
  {"mdecy",             usercmd_mdec,        USER_DEC_Y,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Decrease the Y coordinate.",
   "    See mdecx.\r\n"},
  {"mincy",             usercmd_mdec,        USER_INC_Y,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Increase the Y coordinate.",
   "    See mdecx.\r\n"},
  {"mdecz",             usercmd_mdec,        USER_DEC_Z,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Decrease the Z coordinate.",
   "    See mdecx.\r\n"},
  {"mincz",             usercmd_mdec,        USER_INC_Z,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Increase the Z coordinate.",
   "    See mdecx.\r\n"},
  {"mcoord",            usercmd_mcoord,       0,           USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,   
    "Set the coordinates for current room.",
   "    Usage: mcoord [X] [Y] [Z]\r\n"
   "    Examples: mcoord -2 4 5 / mcoord -2 / mcoord -2 4 / mcoord -2 4 5 \r\n\r\n"
   "Set the coordinates of current rooms to given values.\r\n"},

  {"mdoor",             usercmd_mdoor,   USER_DOOR_NORMAL,  USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Add a door in direction",
    "   Usage: mdoor <doorname|remove> <dirrection>\r\n"
    "   Examples: mdoor stonedoor east\r\n" 
    "             mdoor stonedoor w\r\n"
    "             mdoor remove w\r\n\r\n"
    "If there is no connection in given direction an undefined connection will be created.\r\n"
    "Existing doorname will be removed - so take care.\r\n"
    "   Note that remove has to be written fully and not in capitals.\r\n"},
  {"mexit",             usercmd_mdoor,   USER_DOOR_EXIT,     USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Add a nonsecret door",
    "   Usage: mdoor <doorname> <dirrection>\r\n"
    "    Examples: mdoor stonedoor east / mdoor stonedoor w\r\n\r\n"
    "If there is no connection in given direction an undefined connection will be created.\r\n"
    "Existing doorname will be removed - so take care. Door name \"exit\" is reserved word,\r\n"
    "and basicly means - nonsecret exit. Use \"Exit\" to mark secret door.\r\n"},
  {"mmark",             usercmd_mmark,          0,      USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Mark/Flag some direction.",
    "    Usage: mmark <dir> <flag>\r\n"
    "    Examples: mmark west undefined / mmark w death\r\n\r\n"
    "    Possible flags: undefined, death\r\n"},
  {"mlink",             usercmd_mlink,          0,      USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Link current room with some other.",
    "    Usage: mlink <dirrection> <id> [backdir] [force|oneway]\r\n"
    "    Examples: \r\n"
    "   mlink west 120          link exit werst in current room with exit east in room 120.\r\n"
    "                    Wont work if exit if already busy.\r\n"
    "   mlink e 120 o           link exit west as oneway to room 120.\r\n"
    "   mlink w 120 e force     link west to 120 and force all connections. If exists are busy\r\n"
    "                           they will be overwritten. Doors will stay though.\r\n"
    "   mlink w 120 o force     link west to 120 and force it.\r\n\r\n"
    "    Link two rooms.\r\n"},
  {"mdetach",             usercmd_mdetach,          0,      USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Detach connections in 2 rooms.",
    "    Usage: mdetach <dirrection> [oneway|delete]\r\n"
    "    Examples: \r\n"
    "   mdetach west            detach the link in both rooms and mark exits as undefined.\r\n"
    "   mdetach w one           detach link only in this room.\r\n"
    "   mdetach west delete     detach in both rooms and remove the exits.\r\n"
    "   mdetach w one d         detach link west and delete the exit only in this room.\r\n" 
    "                           Leave oneway in second room.\r\n\r\n"
    "    Disconnect two rooms.\r\n"},
  {"mgoto",               usercmd_mgoto,        0,       USERCMD_FLAG_REDRAW, 
    "Set some room as current (using id or direction).",
    "    Usage: mgoto <id|direction>\r\n"
    "    Examples: mgoto 120 / mgoto west /mgoto w\r\n\r\n"
    "   Set some room as current mappers position. To use direction as argument\r\n"
    "you need to be synced.\r\n"},
  {"maction",               usercmd_maction,        0,       0, 
    "Perform some action with doors in current room.",
    "    Usage: maction [local] <dir|all> <action>\r\n"
    "    Examples: \r\n"
    "     maction east open             open exit(secret) east. will send command\r\n"
    "                                   open <doorname_east> e\r\n"
    "     maction e cast 'block'        will send cast 'block' <door> e to mud\r\n"
    "     maction local all --[ Door:   will show all secret doors with diven prompt\r\n"
    "     maction local e --[EAST:      will send --[EAST: door e to your client only\r\n\r\n"
    "   Performs some actions with doors.\r\n"},

  {"mdebug",               usercmd_mdebug,        0,       0, 
    "Configure debug information/messages.",
    "    Usage: mdebug [debug-option] [boolean]\r\n"
    "    Examples: \r\n"
    "     mdebug                        display current config and available options\r\n"
    "     mdebug analyzer on            allow analyzers debug messages\r\n\r\n"
    "   Configure your debug messages.\r\n"},
  {"mdelete",               usercmd_mdelete,        0,       USERCMD_FLAG_SYNC | USERCMD_FLAG_REDRAW,
    "Delete current room.",
    "    Usage: mdelete [remove]\r\n"
    "   Deletes current room. Option remove forces exits and doors removal in other rooms.\r\n"},
  {"mrefresh",              usercmd_mrefresh,        0,      USERCMD_FLAG_SYNC,
    "Refresh roomdesc.",
    "    Usage: mrefresh \r\n\r\n"
    "   Uses last seen roomdesc. use together with mgoto.\r\n"},
  {"mtreestats",              usercmd_mtreestats,        0,      0,
    "Print tree search engine statistics.",
    "    Usage: mtreestats \r\n\r\n"
    "   Prints tree statistics in terminal window.\r\n"},
  {"mquit",              usercmd_mquit,        0,      0,
    "[DEBUG] quits the analyzer. halts the link etc.",
    "    Usage: mquit \r\n\r\n"
    "   Just zaps the link and quits all threads.\r\n"},

  {"mevent",              usercmd_mevent,        0,      0,
    "Raise an internal event",
    "    Usage: m_event <event|list> [data]\r\n\r\n"
    "   Puts given event in appropriate stack - use together with client actions, but with care.\r\n"
    "This might corrupt events order and thus cause false sync or malfunction. See manual for event"
    "types (or type mevent list) and additional information.\r\n"},
    
    
    
  {NULL, NULL, 0, 0, NULL, NULL}
};


int parse_user_input_line(char *line)
{
  char *p;
  char arg[MAX_STR_LEN];
  int i;
  int parse_result;
  
  p = skip_spaces(line);

    
  if (!*p) {
    return USER_PARSE_NONE;
  }

  
  p = one_argument(p, arg, 0);
    
    
  for (i=0; user_commands[i].name != NULL; i++) 
    if (strcmp(user_commands[i].name, arg) == 0) {
      /* call the appropriate command handler */
      
      if (IS_SET(user_commands[i].flags, USERCMD_FLAG_SYNC)) 
        CHECK_SYNC;
      
      parse_result = ((*user_commands[i].command_pointer) (i, user_commands[i].subcmd, p));
      
      if (IS_SET(user_commands[i].flags, USERCMD_FLAG_REDRAW)) 
        toggle_renderer_reaction();
      
      if (renderer_window)
        renderer_window->update_status_bar();
      
      return parse_result;
    }
  
  if (mud_emulation) {
    send_to_user("Arglebargle...No such command\r\n");
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }    
  
  return USER_PARSE_NONE;
}

void display_debug_settings()
{
  int i;
  
  send_to_user("--[ Debug settings:\r\n");
  for (i = 0; debug_data[i].name; i++) 
    send_to_user("   %-14s %-50s [%-3s]\r\n", debug_data[i].name,
        debug_data[i].desc, ON_OFF(debug_data[i].state));
  
  send_to_user("\r\n");
}

USERCMD(usercmd_mevent)
{
  char *p;
  char arg[MAX_STR_LEN];
  int i;
  int event;
  
  userfunc_print_debug;


  p = skip_spaces(line);
  if (!*p) {
    send_to_user("--[ Missing event for mevent command. (type 'mevent list' for possible events list)\r\n");

    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  p = one_argument(p, arg, 2);

  if (strcmp(arg, "LIST") == 0) {
    send_to_user("--[ Possible events for mevent command: \r\n\r\n");
    
    
    for (i = 0; event_types[i].name; i++) 
      send_to_user("  Event %-10s Group/Type %-10s\r\n", event_types[i].name, 
            event_types[i].group == E_CAUSE ? "CAUSE" : "RESULT" );
    
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }    

  
  
  event = -1;
  for (i = 0; event_types[i].name; i++) 
    if (strcmp(arg, event_types[i].name) == 0) 
    {
      event = event_types[i].type;
      break;
    }
    
  if (event == -1) {
    send_to_user("--[ %s is not an event.\r\n", arg);

    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  p = skip_spaces(p);
  if (!*p) {
    send_to_user("--[ Missing data for mevent command.\r\n");
    
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  p = one_argument(p, arg, 0);

  if (event_types[i].group == E_CAUSE) 
    preCAdd(event, arg);
  else if (event_types[i].group == E_RESULT)
    preRAdd(event, arg);
    
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mdebug)
{
  char *p;
  char arg[MAX_STR_LEN];
  int i, desired;

  userfunc_print_debug;


  p = skip_spaces(line);
  if (!*p) {
    display_debug_settings();
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  p = one_argument(p, arg, 0);
  for (i = 0; debug_data[i].name; i++) 
    if (is_abbrev(arg, debug_data[i].name)) {
      /* found passing debug setting */
      
      p = skip_spaces(p);
      if (!*p) {
        debug_data[i].state = 1 - debug_data[i].state;
        send_to_user("--[ Setting %s to %s.\r\n", debug_data[i].desc, 
                                                  ON_OFF(debug_data[i].state) );
    
        send_to_user(last_prompt);
        return USER_PARSE_DONE;
      }
      p = one_argument(p, arg, 0);
        
      desired = get_input_boolean(arg);
      debug_data[i].state = desired;
      send_to_user("--[  Setting %s to %s.\r\n", debug_data[i].desc, 
                                                ON_OFF(debug_data[i].state) );
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }
  
  display_debug_settings();

  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_maddroom)
{
  struct Troom *r;

  if (mud_emulation) {
    send_to_user("Disabled in MUD emulation.\r\n");
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  if (line) {} /* just to avoid warning about unused variable - its annoying */
  
  userfunc_print_debug;

  if (engine_flags.last_roomname[0] == 0) {
    send_to_user("--[ Missing room name.\r\n");
    
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  if (engine_flags.last_desc[0] == 0) {
    send_to_user("--[ Missing description!\r\n");
  }

  if (engine_flags.last_exits[0] == 0) {
    send_to_user("--[ Missing exits.\r\n");
    
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  send_to_user("--[ (Forced) adding new room!\n");
  print_debug(DEBUG_ANALYZER, "adding new room!");
  

  
  roomer.fixfree();	/* making this call just for more safety - might remove */

  r = new Troom;
  reset_room(r);

  r->id = roomer.next_free;
  r->name = strdup(engine_flags.last_roomname);
  r->desc = strdup(engine_flags.last_desc);
  roomer.add_terrain(r, engine_flags.last_terrain);
  
  
  addedroom = r;
  engine_flags.addingroom = true;
  do_exits(engine_flags.last_exits);
  addedroom = NULL;
  engine_flags.addingroom = false;
  
  r->x = 100;
  r->y = 100;
  r->z = 100;
  
  roomer.addroom(r);
  
  stacker.put(r->id);
  stacker.swap();

  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_maction)
{
  char *p;
  char arg[MAX_STR_LEN];
  struct Troom *r;
  int dir;
  int local;
  unsigned int i;
  char exit;

  userfunc_print_debug;
  
  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS
  p = one_argument(p, arg, 0);

  local = 0;
  if (is_abbrev(arg, "local")) {
    local = 1;
    p = skip_spaces(p);
    if (!*p) MISSING_ARGUMENTS
    p = one_argument(p, arg, 0);
  }

//  PARSE_DIR_ARGUMENT(dir, arg);
  dir = parse_dir(arg);               
  if (dir == -1) {                      
    if (is_abbrev(arg, "all")) {
      dir = -1;
    } else {
      send_to_user("--[ %s is not a dirrection.\r\n", arg);     
      send_to_user(last_prompt); 
      return USER_PARSE_DONE;             
    }
  }                                             
  
  p = skip_spaces(p);
  /* some safety checks */
  strncpy(arg, p, MAX_STR_LEN/2);

  if (stacker.amount == 0) {
    if (dir == -1) {
      send_to_user("--[ undefined position. can not operate with secrets.\r\n");     
      send_to_user(last_prompt); 
      return USER_PARSE_DONE;             
    }
    
    if (!local) 
      send_to_mud("%s exit %s\n", arg, exits[dir]);
      
    send_to_user("%s exit %s\r\n", arg, exits[dir]);
    
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
    
  }
  
  
  exit = 0;             /* this flag is for 'action exit' when there are several rooms */
  
  /* get the door names */
  for (i = 1; i <= stacker.amount; i++) {
    r = roomer.getroom(stacker.get(i));

    if (dir == -1) {
      int z;
      
      for (z = 0; z <= 5; z++)
        if (r->doors[z] != NULL && strcmp(r->doors[z], "exit") != 0 ) {
          if (!local) 
            send_to_mud("%s %s %s\n", arg, r->doors[z] , exits[z]);
          
          send_to_user("%s %s %s\r\n", arg, r->doors[z] , exits[z]);
        }
        
    } else {
      
      if (r->doors[dir] == NULL || ( strcmp(r->doors[dir], "exit") == 0 )) {
        exit = 1;         /* set the flag that 'exit' exit should be opened */
      } else {
        if (!local) 
          send_to_mud("%s %s %s\n", arg, r->doors[dir] , exits[dir]);
        
        send_to_user("%s %s %s\r\n", arg, r->doors[dir] , exits[dir]);
      }
      
    }
    
  }

  
  /* open the exit only once */
  if (exit && dir != -1) {
    if (!local) 
      send_to_mud("%s %s %s\n", arg, "exit", exits[dir]);
      
    send_to_user("%s %s %s\r\n", arg, "exit", exits[dir]);
  }
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mdelete)
{
  char *p;
  char arg[MAX_STR_LEN];
  struct Troom *r;
  int remove;

  userfunc_print_debug;
  
  remove = 0;
  p = skip_spaces(line);
  if (*p) {
    p = one_argument(p, arg, 0);
    if (is_abbrev(arg, "remove"))
      remove = 1;
    
  } 
  
  r = roomer.getroom(stacker.get(1));

  if (remove) 
    roomer.delete_room(r, 0);
  else 
    roomer.delete_room(r, 1);
  
  stacker.swap();
  
  send_to_user("--[ Removed.\r\n");
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mtreestats)
{

  userfunc_print_debug;

  skip_spaces(line);


  namer.print_tree_stats();
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mrefresh)
{
  userfunc_print_debug;
  skip_spaces(line);

  roomer.refresh_roomname(stacker.get(1), engine_flags.last_roomname);
  roomer.refresh_desc(stacker.get(1), engine_flags.last_desc);
  roomer.refresh_terrain(stacker.get(1), engine_flags.last_terrain); 

  roomer.room_modified( roomer.getroom( stacker.get(1) ) );
  
  send_to_user("--[ Refreshed.\r\n");
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mgoto)
{
  char *p;
  char arg[MAX_STR_LEN];
  unsigned int id;
  struct Troom *r;
  int dir;
  
  userfunc_print_debug;
  
  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS

  p = one_argument(p, arg, 0);
  if (is_integer(arg)) {
    id = atoi(arg);
    if (roomer.getroom(id) == NULL) {
      send_to_user("--[ There is no room with id %s.\r\n", arg);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }
      
    stacker.put(id);
    stacker.swap();
  } else {
    
    CHECK_SYNC;
    r = roomer.getroom(stacker.get(1));
    
    PARSE_DIR_ARGUMENT(dir, arg);

    if ( !roomer.is_connected(r, dir) ) {
      send_to_user("--[ Bad direction - there is no connection.\r\n", arg);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }

    stacker.put(r->exits[dir]);
    stacker.swap();
  }
    
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


/*     "    Usage: mdetach <direction> [oneway|delete]\r\n" */
USERCMD(usercmd_mdetach)
{
  char *p;
  char arg[MAX_STR_LEN];
  struct Troom *r, *s;
  int del = 0, oneway = 0;
  int dir;
  unsigned int i;
  
  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS
    
  p = one_argument(p, arg, 0);

  PARSE_DIR_ARGUMENT(dir, arg);
  
  for (i = 0; i < 2; i++) {
    p = skip_spaces(p);
    if (*p) {
      p = one_argument(p, arg, 0);
      if (is_abbrev(arg, "oneway") )
        oneway = 1;
      if (is_abbrev(arg, "delete") )
        del = 1;
    }
  }
  
  r = roomer.getroom(stacker.get(1));
  if (r->exits[dir] != 0) {
    s = roomer.getroom(r->exits[dir]);
    if (del) {
      if (r->doors[dir] != NULL)
        roomer.remove_door(r, dir);
      r->exits[dir] = 0;
    } else {
      r->exits[dir] = EXIT_UNDEFINED;
    }
  } else {
    send_to_user("--[ %s is not marked nor linked.\r\n", exits[dir] );
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  roomer.room_modified(r);
  
  if (!oneway && s!=NULL) {
    for (i = 0; i<= 5; i++) 
      if (s->exits[i] == r->id) {
        if (del) {
          s->exits[i] = 0;
          if (r->doors[i] != NULL)
            roomer.remove_door(s, i);
        } else {
          s->exits[i] = EXIT_UNDEFINED;
        }
          
      }
  }
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}

/* "    Usage: mlink <dirrection> <id> [backdir] [force|oneway]\r\n"  */
USERCMD(usercmd_mlink)
{
  int force = 0, oneway = 0, backdir = -1, id, dir;
  char *p;
  char arg[MAX_STR_LEN];
  struct Troom *r, *second;
  int i;

  userfunc_print_debug;

  r = roomer.getroom(stacker.get(1));
  
  
  /* get essential arguments - id and direction */
  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS
    
  p = one_argument(p, arg, 0);
  
  
  PARSE_DIR_ARGUMENT(dir, arg);
    
  p = skip_spaces(p);
  if (!*p) MISSING_ARGUMENTS
  p = one_argument(p, arg, 0);

  GET_INT_ARGUMENT(arg, id);

  second = roomer.getroom(id);
  
  if (second == NULL) {
    send_to_user("--[ There is no room with id %i.\r\n", id);
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  /* get optional arguments */
  do {
    p = skip_spaces(p);
    p = one_argument(p, arg, 0);
    
    for (i = 0; i <= 5; i++)
      if (is_abbrev(arg, exits[i]) ) 
        backdir = i;

    if (is_abbrev(arg, "force"))
      force = 1;
    
    if (is_abbrev(arg, "oneway"))
      oneway = 1;
    
    
  } while (*p);
  
  /* first room */
  if (roomer.is_connected(r, dir) && !force) {
    send_to_user("--[ There is an existing connection to the %s.\r\n", exits[dir]);
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  r->exits[dir] = id;
  send_to_user("--[ Linked exit %s with %s [%i].\r\n", exits[dir], second->name, id);

  roomer.room_modified(r);
  
  /* if oneway option was given - we are done here */
  if (!oneway) {
    /* setup backdir to dummie-opposite if needed */
    if (backdir == -1) 
      backdir = reversenum(dir);
    
    if (roomer.is_connected(second, backdir) && !force) {
      send_to_user("--[ There is an existing connection to the %s in second room.\r\n", exits[backdir]);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }
    
    
    second->exits[backdir] = r->id;
    send_to_user("--[ Linked exit %s in second room with this room.\r\n", exits[backdir]);
      
  }
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mmark)
{
  char *p;
  char arg[MAX_STR_LEN];
  struct Troom *r;
  int i, dir;

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS
  
  p = one_argument(p, arg, 0);    /* direction */
  
  PARSE_DIR_ARGUMENT(dir, arg);
  
  p = skip_spaces(p);
  if (!*p) MISSING_ARGUMENTS

  p = one_argument(p, arg, 0);    /* flag */
  
  r = roomer.getroom(stacker.get(1));

  
  i = 0;
  while (room_flags[i].name) { 
    if (is_abbrev(arg, room_flags[i].name) ) {
      r->exits[dir] = room_flags[i].flag;
      send_to_user("--[Pandora: Marked %s as %s\r\n", exits[dir],  room_flags[i].xml_name);
      
      roomer.room_modified(r);
      
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }
    i++;
  }
  send_to_user("--[ no such flag.\r\n");
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}

USERCMD(usercmd_mdoor)
{
  char *p;
  char arg[MAX_STR_LEN];
  char arg2[MAX_STR_LEN];
  struct Troom *r;
  int i;

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) MISSING_ARGUMENTS
  
  switch (subcmd)
  {
	case  USER_DOOR_NORMAL:
                p = one_argument(p, arg, 1);    /* do not lower-case the door name */
                
                p = skip_spaces(p);
                if (!*p) MISSING_ARGUMENTS
                
		break;
	case  USER_DOOR_EXIT:
                strcpy(arg, "exit");             /* normal exit */
                
		break;
  }
  
  p = one_argument(p, arg2, 0);    /* direction */
  
  r = roomer.getroom(stacker.get(1));
  
  PARSE_DIR_ARGUMENT(i, arg2);
  
  if (strcmp(arg, "remove") == 0) {
    if (r->doors[i] == NULL) {
      send_to_user("--[ There is no door to the %s.\r\n", exits[i]);
    } else {
      roomer.remove_door(r, i);
      send_to_user("--[Pandora: Removed the door to the %s\n", exits[i]);
    }
  } else {
    roomer.add_door(r, arg, i);
    send_to_user("--[Pandora: Added the door %s to the %s\n", arg, exits[i]);
  }
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}



USERCMD(usercmd_mcoord)
{
  char *p;
  char arg[MAX_STR_LEN];
  int value;
  struct Troom *r;
  
  userfunc_print_debug;

  r = roomer.getroom(stacker.get(1));
    
  p = skip_spaces(line);
  if (!*p) {
    send_to_user("--[ Current coordinates : X %i, Y %i, Z %i.\r\n", r->x, r->y, r->z);
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  p = one_argument(p, arg, 0);
  
  GET_INT_ARGUMENT(arg, value);
  r->x = value;
  send_to_user("--[ X set to %i.\r\n", r->x);
  
  p = skip_spaces(p);
  if (!*p) {
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  p = one_argument(p, arg, 0);
  
  GET_INT_ARGUMENT(arg, value);
  r->y = value;
  send_to_user("--[ Y set to %i.\r\n", r->y);
  
  p = skip_spaces(p);
  if (!*p) {
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  p = one_argument(p, arg, 0);
  GET_INT_ARGUMENT(arg, value);
  r->z = value;
  send_to_user("--[ Z set to %i.\r\n", r->z);

  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}



USERCMD(usercmd_mdec)
{
  char *p;
  char arg[MAX_STR_LEN];
  int value;
  struct Troom *r;
  
  userfunc_print_debug;

  value = 1;
  p = skip_spaces(line);
  if (*p) {
    p = one_argument(p, arg, 0);
    
    GET_INT_ARGUMENT(arg, value);
  }

  r = roomer.getroom(stacker.get(1));
  
  switch (subcmd)
  {
	case  USER_DEC_X:
                roomer.setx(stacker.get(1), r->x - value);
		break;
	case  USER_INC_X:
                roomer.setx(stacker.get(1), r->x + value);
		break;
	case  USER_DEC_Y:
                roomer.sety(stacker.get(1), r->y - value);
		break;
	case  USER_INC_Y:
                roomer.sety(stacker.get(1), r->y + value);
		break;
	case  USER_DEC_Z:
                roomer.setz(stacker.get(1), r->z - value);
		break;
	case  USER_INC_Z:
                roomer.setz(stacker.get(1), r->z + value);
		break;
  }

  send_to_user("--[Pandora: Changed.\n");
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mnewmap)
{
  userfunc_print_debug;
  skip_spaces(line);

  send_to_user("--[ Disabled. Use maddroom instead.\r\n");
  send_to_user(last_prompt);
  return USER_PARSE_DONE;

  
  
  if (mud_emulation) {
    send_to_user("Disabled in MUD emulation.\r\n");
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }
  
  
  if (roomer.amount == 0) {
    engine_flags.gettingfirstroom = 1;
    
    send_to_mud("examine\n");
  } else {
    send_to_user("Current database is not empty!\n");
  }


  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}

/* print brief help if no arguments given, or detailed help for given argument*/
USERCMD(usercmd_mhelp)
{
  char *p;
  char arg[MAX_STR_LEN];
  
  userfunc_print_debug;

  p = skip_spaces(line);
  if (*p) {
    /* argument is given */
    p = one_argument(p, arg, 0);        
    
    for (int i=0; user_commands[i].name != NULL; i++) 
      if ((strcmp(user_commands[i].name, arg) == 0) && user_commands[i].help)
      {
        send_to_user("---[Help file : %s.\r\n", user_commands[i].name);
        
        send_line_to_user((char *)user_commands[i].help);
        
        send_to_user("\r\n");
        
        send_to_user(last_prompt);

        return USER_PARSE_DONE;
      }
    
  }
  
  /* else we print all brief help files */
  
  send_to_user("----[ Brief help files/commands overview.\r\n\r\n");
  for (int i=0; user_commands[i].name != NULL; i++) 
    if (user_commands[i].desc)
      send_to_user("  %-15s  %-65s\r\n", user_commands[i].name, user_commands[i].desc);
  
  send_to_user("\r\n");

  send_to_user(last_prompt);
    
  return USER_PARSE_DONE;
}

USERCMD(usercmd_config)
{
  int desired;
  char *p;
  char arg[MAX_STR_LEN];

  userfunc_print_debug;

  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
    desired = -1;       /* so, toggle the state */
  } else {
    /* else get the desired state */
    
    p = one_argument(p, arg, 0);
    
    desired = get_input_boolean(arg);
  }
  
  switch (subcmd)
  {
	case  USER_CONF_MAP :
                if (desired == -1) {
                  
                }
        
                if ((desired == 0) || (desired == -1 && engine_flags.mapping == 1)) {
                    engine_flags.mapping = 0;
                    send_to_user("--[Pandora: Mapping is now OFF!\r\n");
            
                    send_to_user(last_prompt);
                    return USER_PARSE_DONE;
                } else if (desired == 1 || (desired == -1 && engine_flags.mapping == 0) )
                  
                {
                  if (Cbottom != Ctop || Rbottom != Rtop) {
                    send_to_user("--[Pandora: mreset is required to clear the Cause-Result stacks.\n");
                    send_to_user(last_prompt);
            
                    return USER_PARSE_DONE;
                  }
                    
                  engine_flags.mapping = 1;
                  engine_flags.exits_check = 0;
                  send_to_mud("brief OFF\n");
                  send_to_mud("spam ON\n");
                  send_to_mud("prompt all\n");
        
                  send_to_user("--[Pandora: Exits analyzer is now OFF\n");
                  send_to_user("--[Pandora: Mapping is now on!\n");

                  send_to_user(last_prompt);
            
                  return USER_PARSE_DONE;
                }
                
		break;
	case  USER_CONF_BRIEF:
                if (desired == -1)
                  dispatcher.set_brief_mode( 1 - dispatcher.get_brief_mode());
                else 
                  dispatcher.set_brief_mode(desired);
                
                send_to_user("----[ Mapper Brief Mode is now %s.\r\n", 
                              ON_OFF(dispatcher.get_brief_mode()) );
		break;
	case  USER_CONF_AUTOMERGE:
                if (desired == -1)
                  engine_flags.automerge = 1 - engine_flags.automerge;
                else 
                  engine_flags.automerge = desired;
                
                send_to_user("----[ Description analyzer and automatic merging is now %s.\r\n", 
                              ON_OFF(engine_flags.automerge) );
		break;
	case  USER_CONF_ANGRYLINKER:
                if (desired == -1)
                  engine_flags.angrylinker = 1 - engine_flags.angrylinker;
                else 
                  engine_flags.angrylinker = desired;
                
                send_to_user("----[ AngryLinker is now %s.\r\n", 
                              ON_OFF(engine_flags.angrylinker) );
		break;

        case  USER_CONF_EXITS:
                if (desired == -1)
                  engine_flags.exits_check = 1 - engine_flags.exits_check;
                else 
                  engine_flags.exits_check = desired;
                
                send_to_user("----[ Exits analyzer is now %s.\r\n", 
                              ON_OFF(engine_flags.exits_check) );
		break;
	case  USER_CONF_TERRAIN:
                if (desired == -1)
                  engine_flags.terrain_check = 1 - engine_flags.terrain_check;
                else 
                  engine_flags.terrain_check = desired;
                
                send_to_user("----[ Terrain analyzer is now %s.\r\n", 
                              ON_OFF(engine_flags.terrain_check) );
		break;
  }

  send_to_user(last_prompt);

  return USER_PARSE_DONE;
}


USERCMD(usercmd_msave)
{
  char *p;
  char arg[MAX_STR_LEN];
  
  userfunc_print_debug;
  
  
  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
    xml_writebase(base_file);
    send_to_user("--[Pandora: Saved...\r\n");
    modified = 0;

    
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  } else {
    p = one_argument(p, arg, 1);        /* do not lower or upper case - filename */

    xml_writebase(arg);
    send_to_user("--[Pandora: Saved to %s...\r\n", arg);
    
    modified = 0;

    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }  
}  
  
USERCMD(usercmd_mload)
{
  char *p;
  char arg[MAX_STR_LEN];

  userfunc_print_debug;

  send_to_user("--[Pandora: Reloading the database ...\n");

  send_to_user(" * Locking analyzer-thread...\r\n");
  analyzer_mutex.lock();

  send_to_user(" * Re-initing roomer and namer classes...\r\n");
  roomer.reinit();  /* this one reinits Ctree structure also */
  
  send_to_user(" * Resetting possibility stacks...\r\n");
  stacker.reset();  /* resetting stacks */
  
  send_to_user(" * Clearing events stacks...\r\n");
  clear_events_stacks();
  
  addedroom = NULL;       /* possible memory leak, but who cares ... */
  engine_flags.mapping = 0;
  

  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
    send_to_user(" * Loading the base %s from the disk...\r\n", base_file);
    xml_readbase(base_file);
  } else {
    p = one_argument(p, arg, 1);        /* do not lower or upper case - filename */

    send_to_user(" * Loading the base %s from the disk...\r\n", arg);
    xml_readbase(arg);
  }  


  send_to_user(" * Ready to go!\r\n");
  
  analyzer_mutex.unlock();
  
  send_to_user(" * Unlocking analyzer-thread...\r\n");
  
  send_to_user("--[Pandora: Done.\r\n");

  modified = 0;

  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}


USERCMD(usercmd_mreset)
{
  userfunc_print_debug;
  skip_spaces(line);

  while (Cbottom != Ctop)
      Cremove();
  while (Rbottom != Rtop)
      Rremove();

  stacker.reset();

  send_to_user("--[Pandora: Resetted!\n");
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}

USERCMD(usercmd_mmerge)
{
  char *p;
  char arg[MAX_STR_LEN];
  int force = 0;
  struct Troom *t;
  int j = -1, i;
  unsigned int id;
  
  userfunc_print_debug;

  if (addedroom == NULL) {
    send_to_user("--[There is new added room to merge.\r\n");
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }

  /* find the only defined exit in new room - the one we came from */
  for (i = 0; i <= 5; i++)
    if (addedroom->exits[i] > 0 && addedroom->exits[i] < MAX_ROOMS) {
      j = i;
      break;
    }

  
  p = skip_spaces(line);
  if (!*p) {
    /* no arguments at all - so, find ID */

    id = EXIT_UNDEFINED;
    
    t = roomer.getroom(0);
    t = t->next;
    while (t != NULL) {
        if (addedroom->id == t->id) {
            t = t->next;
            print_debug(DEBUG_ANALYZER, "Attempted to merge the same rooms, skipped.");
            continue;
        }
        if (t->desc == NULL) {
          t = t->next;
          continue;
        }
        if (t->name == NULL) {
          printf("Analyzer: room %i has no roomname!\r\n", t->id);
          t = t->next;
          continue;
        }
        
        /* in this case we do an exact match for both roomname and description */
        if (strcmp(addedroom->desc, t->desc) == 0) {
          if (strcmp(addedroom->name, t->name) == 0) {
            id = t->id;
            break;
          }	
        }
        t = t->next;
    }

    
    if (id == EXIT_UNDEFINED) {
      send_to_user("--[ No matching room found.\r\n");
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }
    
  } else {
    
    p = one_argument(p, arg, 0);
    GET_INT_ARGUMENT(arg, id);
    
    if (id <= 0 || id > MAX_ROOMS) {
      send_to_user("--[ %s is not a room id.\r\n", arg);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }

    t = roomer.getroom(id);
    if (t == NULL) {
      send_to_user("--[ There is no room with this id %i.\r\n", id);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }      

    p = skip_spaces(p);
    if (*p) {
      p = one_argument(p, arg, 0);
      if (is_abbrev(arg, "force"))
        force = 1;
      
    }
    
    if (!force) 
      if ((strcmp(addedroom->desc, t->desc) != 0) || 
          (strcmp(addedroom->name, t->name) != 0) ) 
      {
        send_to_user("--[ Roomname or description do not match.\r\n");
        send_to_user(last_prompt);
        return USER_PARSE_DONE;
      }
    
  } 

  analyzer_mutex.lock();
    
  if (roomer.try_merge_rooms(t, addedroom, j)) {
    send_to_user("--[ merged.\r\n");
  } else {
    send_to_user("--[ failed.\r\n");
    stacker.put(addedroom->id);
  }

  analyzer_mutex.unlock();

  stacker.swap();

  /* now make sure we have a room in stack */
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}



USERCMD(usercmd_mstat)
{
  userfunc_print_debug;
  skip_spaces(line);

  printstacks();
  
  send_to_user(last_prompt);
  return USER_PARSE_DONE;
}



USERCMD(usercmd_minfo)
{
  char *p;
  char arg[MAX_STR_LEN];
  int id;
  struct Troom *t;
  unsigned int i;
  
  userfunc_print_debug;
  
  p = skip_spaces(line);
  if (!*p) {
    /* no arguments */
  } else {
    p = one_argument(p, arg, 1);        /* do not lower or upper case - filename */

    GET_INT_ARGUMENT(arg, id);

    if (id <= 0 || id > MAX_ROOMS) {
      send_to_user("--[ %s is not a room id.\r\n", arg);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }
      
    t = roomer.getroom(id);
    if (t == NULL) {
      send_to_user("--[ There is no room with this id %i.\r\n", id);
      send_to_user(last_prompt);
      return USER_PARSE_DONE;
    }      
    
    roomer.send_room(t);
    send_to_user(last_prompt);
    return USER_PARSE_DONE;
  }  

  

   for (i = 1; i <= stacker.amount; i++) {
     t = roomer.getroom(stacker.get(i));
     roomer.send_room(t);
   }
   
   send_to_user(last_prompt);
   return USER_PARSE_DONE;
}


USERCMD(usercmd_move)
{
  char *p;
  struct Troom *r;
  int dir;

  userfunc_print_debug;

  /*firstly - send the command futher to mud, as if it matches */
  /* used commands - it wont be automatically send futher */
  
  if (!mud_emulation) {
    
  
    switch (subcmd)
    {
          case  NORTH:
                  preCAdd(C_MOVE, "north");
                  break;
          case  EAST:
                  preCAdd(C_MOVE, "east");
                  break;
          case  SOUTH:
                  preCAdd(C_MOVE, "south");
                  break;
          case  WEST:
                  preCAdd(C_MOVE, "west");
                  break;
          case  UP:
                  preCAdd(C_MOVE, "up");
                  break;
          case  DOWN:
                  preCAdd(C_MOVE, "down");
                  break;
          case USER_MOVE_LOOK:
                  p = skip_spaces(line);
                  if (!*p) {      
                    preCAdd(C_LOOK, NULL);
                  }
                  
                  break;
    }      
  } else {
    r = roomer.getroom(stacker.get(1));

    dir = -1;
    
    switch (subcmd)
    {

          case  NORTH:
                  dir = NORTH;
                  break;
          case  EAST:
                  dir = EAST;
                  break;
          case  SOUTH:
                  dir = SOUTH;
                  break;
          case  WEST:
                  dir = WEST;
                  break;
          case  UP:
                  dir = UP;
                  break;
          case  DOWN:
                  dir = DOWN;
                  break;
          case USER_MOVE_LOOK:
                  roomer.send_room(r);
                  send_to_user(last_prompt);
                  return USER_PARSE_DONE;

                  break;
    }      
    
    if (dir == -1) 
      return USER_PARSE_NONE;
    if ( !roomer.is_connected(r, dir) ) {
      send_to_user("Alas, you cannot go this way.\r\n\r\n");
    } else {
      stacker.put(r->exits[dir]);
      stacker.swap();
    
      r = roomer.getroom(stacker.get(1));
      toggle_renderer_reaction();
    
      roomer.send_room(r);
    }
            
    
    send_to_user(last_prompt);
    
    return USER_PARSE_DONE;
    
  }
  
  return USER_PARSE_NONE;
}


USERCMD(usercmd_mquit)
{
  userfunc_print_debug;
  skip_spaces(line);

  proxy_shutdown();
  
  exit(1);    
  
    
  return USER_PARSE_NONE;
}
