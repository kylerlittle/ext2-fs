#include "my_pwd.h"

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

int rpwd(MINODE *wd)
{
  char buf[BLKSIZE], myname[256], *cp;
  MINODE *parent, *ip;
  u32 myino, parentino;
  DIR *dp;

  if (wd == root) return 0;

  parentino = findino(wd, &myino);
  parent = iget(dev, parentino);

  findmyname(parent, myino, myname);
  // recursively call rpwd()
  rpwd(parent);

  iput(parent);
  printf("/%s", myname);

  return 0;
}

char* pwd(MINODE *wd)
{
  if (wd == root)
  {
    printf("/\n");
    return 0;
  }
  rpwd(wd);
  printf("\n");
  return 0;
}

int my_pwd(int argc, char *argv[]) {
    pwd(running->cwd);
}