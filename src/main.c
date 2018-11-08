#include "util/type.h"
#include "util/util.h"
#include "init.h"

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

int main(int argc, char *argv[]) {
    if (argc > 1) {
        disk = argv[1];
    }
    open_dev(disk);
    dev = fd;
    init();
    mount_root(disk);

    return 0;
}