/*************** cmd.h file *********************************/
#ifndef __CMD__
#define __CMD__

/**** Included headers/libraries ****/
#include "../util/util.h"
#include "level1/my_ls.h"

// int (*fs_cmds[])(int, char **);

/**** Function Prototypes ****/
int get_cmd_index(char *cmd);

typedef struct command {
    char *command_as_string;  // char * since const
    int (*command_as_function)(int, char **);  // function should accept argc and argv
} CMD;

CMD *cmd_table;

#endif
/*********************************************************/