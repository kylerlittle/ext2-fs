#include "my_quit.h"

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
    exit(0);
}