/*************** cmd.h file *********************************/
#ifndef __CMD__
#define __CMD__

/**** Included headers/libraries ****/
#include "level1/my_ls.h"
#include "level1/my_quit.h"
#include "level1/my_pwd.h"
#include "level1/my_chdir.h"
#include "level1/my_link.h"
#include "level1/my_symlink.h"
#include "level1/my_unlink.h"
#include "level1/my_mkdir.h"
#include "level1/my_rmdir.h"
#include "level1/my_creat.h"
#include <stdio.h>
#include <string.h>

/**** Function Prototypes ****/
int get_cmd_index(char *cmd);

typedef struct command {
    char *command_as_string;  // char * since const
    int (*command_as_function)(int, char **);  // function should accept argc and argv
} CMD;

/* Table with cmd string and function address (CMD struct) as entries. */
CMD cmd_table[];

#endif
/*********************************************************/
