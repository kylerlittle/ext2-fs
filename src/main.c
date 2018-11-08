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
int n;

int  fd, dev;
int  nblocks, ninodes, bmap, imap, iblk, inode_start;
char line[256], cmd[32], pathname[256];
char *disk = "mydisk";

int input_help() {
    printf("");
}

void get_input(char *cmd, char *path) {
  char line[MAX_INPUT_LEN];
  printf(">> ");
  fgets(line, MAX_INPUT_LEN, stdin);
  line[strlen(line) - 1] = '\0';
  sscanf(line, "%s %s", cmd, path);
  if (!strcmp(cmd, line)) strcpy(path, "");
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
        get_input(_cmd, _path);
        if (DEBUG_MODE) printf("cmd: %s\tpath: %s\n", _cmd, _path);
        /* Find the index of the command in the table. */
        int index = find_cmd(_cmd);
        
        /* Execute command if valid. Otherwise, print error message. */
        if (is_valid_cmd(index))
            cmd_ptrs[index](_path);
        else
            printf("%s: command not found\n", _cmd);
    }

    return 0;
}