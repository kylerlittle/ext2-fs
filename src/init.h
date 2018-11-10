/*************** init.h file *********************************/
#ifndef __FS_INIT__
#define __FS_INIT__

/**** Included headers/libraries ****/
#include "util/type.h"
#include "util/util.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk, inode_start;
extern char line[MAX_INPUT_LEN], cmd[32], pathname[256];

/**** Function Prototypes ****/
int init();
int open_dev(char *device);
int super(char *buf);
int gd(char *buf);
int init_start_proc();
int mount_root(char *disk);

#endif
/*********************************************************/