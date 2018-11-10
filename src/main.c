#include "util/type.h"
#include "util/util.h"
#include "init.h"
#include "cmd/cmd.h"

/*************** globals for Level-1 ********************/
MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;

char gpath[MAX_FILENAME_LEN];    // holder of component strings in pathname
char *name[MAX_COMPONENTS];     // assume at most 64 components in pathnames
char *cmd_argv[MAX_CMDS];
int n;

int  fd, dev;
int  nblocks, ninodes, bmap, imap, iblk, inode_start;
char line[MAX_INPUT_LEN], cmd[32], pathname[MAX_FILENAME_LEN];
char *disk = "mydisk";

extern CMD cmd_table[];

int input_help() {
    printf("input command : [ls|cd|pwd|quit] ");
}

void get_input(char *line) {
  input_help();
  fgets(line, MAX_INPUT_LEN, stdin);
  line[strlen(line) - 1] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        disk = argv[1];
    }
    open_dev(disk);         // opens disk; sets fd equal to returned fd
    dev = fd;               // set global dev to fd
    init();                 // initialize proc[], root to NULL, and minode[]
    mount_root(disk);       // verify ext2 fs, store globals from super/gd block, & start P0/P1 processes

    /* cmd loop */
    while (1) {
        /* Snag user input. */
        get_input(line);
        if (line[0] == '\0')
            continue;   // user entered nothing; go to next iteration

        /* Tokenize. */
        int n = tokenize(cmd_argv, line, " ");

        /* Find the index of the command in the table. */
        int index = get_cmd_index(cmd_argv[0]);
        
        /* Execute command if valid. Otherwise, print error message. */
        if (index != -1)
            cmd_table[index].command_as_function(n-1, &cmd_argv[1]);
        else
            printf("%s: command not found\n", cmd_argv[0]);

        /* Clear command token list */
        clear_tok_list(cmd_argv);
    }

    return 0;
}