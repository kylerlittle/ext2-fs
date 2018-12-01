/*************** util.h file *********************************/
#ifndef __FS_UTILS__
#define __FS_UTILS__

/**** Included headers/libraries ****/
#include "type.h"

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
int tokenize(char *tok_list[], char *input_line, char *delimiters);
int clear_tok_list(char *tok_list[]);
int clear_toks_after_i(char *tok_list[], int i);
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(int *dev, char *pathname);
int findmyname(MINODE *parent, u32 myino, char *myname);
int findino(MINODE *mip, u32 *myino);
int enter_name(MINODE* mip, int myino, char* myname);
#endif
/*********************************************************/
