#ifndef __RMDIR_H__
#define __RMDIR_H__

#include "../../util/util.h" //will be included in header

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

int my_rmdir(int argc, char *argv[]);
int isEmpty(MINODE *minodePtr);
int sw_kl_rmdir(char *path);

#endif