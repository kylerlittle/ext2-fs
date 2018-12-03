#include "my_quit.h"

/**** globals defined in main.c file ****/
MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
char gpath[MAX_FILENAME_LEN];
char *name[MAX_COMPONENTS];
int n;
int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;
char line[MAX_INPUT_LEN], cmd[32], pathname[MAX_FILENAME_LEN];

int my_quit(int argc, char *argv[]) {
    int i;
    MINODE *mip;
    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount > 0 && mip->dirty) iput(mip);
    }
    /* Since we syscall exit right here, need to free up the mallocated memory.
    Original argv started at one index less than current (since argv excludes cmd);
    thus, simple decrement pointer, then clear_tok_list. */
    char **original_argv = --argv;
    // i = 0;
    // while (argv[i]) printf("freeing up: %s\n", original_argv[i++]);
    clear_tok_list(original_argv);
    // Lastly, deallocate any open file descriptors
    for (i=0; i<NFD; i++) {
        if (running->fd[i] == NULL) continue;
        free(running->fd[i]);
    }
    exit(0);
}