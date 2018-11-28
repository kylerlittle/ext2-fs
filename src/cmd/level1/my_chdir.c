#include "my_chdir.h"

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

int my_chdir(int argc, char *argv[])
{
  char temp[256];
  char buf[BLKSIZE];
  DIR *dp;
  MINODE *ip, *newip, *cwd;
  int dev, ino;
  char c;

  if (argv[0] == NULL)
  {
    iput(running->cwd);
    running->cwd = iget(root->dev, 2);
    return;
  }

  if (strcmp(argv[0], "/") == 0)
    dev = root->dev;
  else
    dev = running->cwd->dev;
  strcpy(temp, argv[0]);
  ino = getino(&dev, temp);

  if (!ino)
  {
    printf("cd : no such directory\n");
    return (-1);
  }
  printf("dev=%d ino=%d\n", dev, ino);
  newip = iget(dev, ino); /* get inode of this ino */

  printf("mode=%4x\n", newip->INODE.i_mode);
  //if ( (newip->INODE.i_mode & 0040000) == 0){
  if (!S_ISDIR(newip->INODE.i_mode))
  {
    printf("%s is not a directory\n", argv[0]);
    iput(newip);
    return (-1);
  }

  iput(running->cwd);
  running->cwd = newip;

  printf("after cd : cwd = [%d %d]\n", running->cwd->dev, running->cwd->ino);
}