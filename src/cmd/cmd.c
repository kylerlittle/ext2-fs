#include "cmd.h"

/* Table of 'command' structs. Each struct contains a const char *, 
pointing at the name of the command, and then the address of the
function which executes the command. All functions will accept argc (int)
and *argv[] since many commands can accept an arbitrary number of parameters.
*/
CMD cmd_table[] = {
    {"ls",          &my_ls},
    {"quit",        &my_quit},
    {"pwd",         &my_pwd},
    {"cd",          &my_chdir},
    {"link",        &my_link},
    {"symlink",     &my_symlink},
    {"unlink",      &my_unlink},
    {"mkdir",       &my_mkdir},
    {"rmdir",       &my_rmdir},
    {"creat",       &my_creat},
    {"chown",       &my_chown},
    {"chmod",       &my_chmod},
    {"touch",       &my_touch},
    {"stat",        &my_stat},
    {"pfd",         &my_pfd},
    {"open",        &my_open},
    {"lseek",       &my_lseek},
    {"close",       &my_close},
    {"write",       &my_write},
    {"read",        &my_read},
    {"cp",          &my_cp},
    {"cat",         &my_cat},
    {"mv",          &my_mv},
    {NULL,          NULL}
};

int get_cmd_index(char *cmd) {
    int i = 0;
    while (cmd_table[i].command_as_string) {
        if (strcmp(cmd, cmd_table[i].command_as_string) == 0) return i;
        i++;
    }
    return -1;
}
