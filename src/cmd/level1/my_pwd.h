#ifndef __PWD__
#define __PWD__

/**** Included headers/libraries ****/
#include "../../util/util.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char gpath[MAX_FILENAME_LEN];
extern char *name[MAX_COMPONENTS];
extern int n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[MAX_INPUT_LEN], cmd[32], pathname[MAX_FILENAME_LEN];

/**** Function Prototypes ****/
int rpwd(MINODE *wd);
char* pwd(MINODE *wd);
int my_pwd(int argc, char *argv[]);

#endif