#include "my_chmod.h"

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

/* Change mode of file (argv[0]) to number specified by argv[1]. */
int my_chmod(int argc, char *argv[])
{
  if (argc < 2) {
    printf("my_chmod: not enough args supplied\n");
  }
  
  int chmod_dev;
  u32 chmod_ino;
  MINODE *chmod_mip;

  int new_mod = atoi(argv[1]);

  // get correct device for argv[0] file
  if (argv[0][0] == '/') chmod_dev = root->dev;
  else chmod_dev = running->cwd->dev;

  // get argv[0] inode into memory
  chmod_ino = getino(&chmod_dev, argv[0]);
  if (!chmod_ino) {
    printf("my_chmod: %s doesn't exist\n", argv[0]);
    return -1;
  }

  // Get argv[0] minode
  chmod_mip = iget(chmod_dev, chmod_ino);

  // Check if current running user has appropriate permissions
  if (running->uid == chmod_mip->INODE.i_uid || running->uid == 0) {
    chmod_mip->INODE.i_mode = new_mod;
    chmod_mip->dirty = 1;
  } else {
    printf("my_chmod: user %d doesn't have privilege to change mode of file %s\n", running->uid, argv[0]);
    iput(chmod_mip);
    return -1;
  }
  
  iput(chmod_mip);
  printf("my_chmod: successfully changed mode of file\n");
  return 0;
}