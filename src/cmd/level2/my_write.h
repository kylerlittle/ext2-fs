#ifndef __WRITE__
#define __WRITE__

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
int strip_quotes(char *dst, char *src);
int my_write(int argc, char *argv[]);
int sw_kl_write(int fd, char buf[], int nbytes);
int get_text(char *buf, int argc, char *argv[]);

#endif