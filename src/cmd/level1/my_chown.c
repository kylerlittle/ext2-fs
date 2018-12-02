#include "my_chown.h"

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

/* Change owner of file (argv[0]). argv[1] is uid. argv[2] is gid. */
int my_chown(int argc, char *argv[])
{
  if (argc < 1) {
    printf("my_chown: not enough args supplied\n");
  }
  
  int chown_dev;
  u32 chown_ino;
  MINODE *chown_mip;

  // IGNORING links for now!
  int new_uid, new_gid;
  if (argc >= 2) new_uid = atoi(argv[1]);
  if (argc >= 3) new_gid = atoi(argv[2]);

  // get correct device for argv[0] file
  if (argv[0][0] == '/') chown_dev = root->dev;
  else chown_dev = running->cwd->dev;

  // get argv[0] inode into memory (since we need its ino and to update it); recall getino tokenizes for us
  chown_ino = getino(&chown_dev, argv[0]);
  if (!chown_ino) {
    printf("my_chown: %s doesn't exist\n", argv[0]);
    return -1;
  }

  // Get argv[0] minode
  chown_mip = iget(chown_dev, chown_ino);

  // Check if current running user has appropriate permissions
  if (running->uid == new_uid || running->uid == 0) {
    chown_mip->INODE.i_uid = new_uid;
    chown_mip->dirty = 1;
  } else {
    printf("my_chown: user %d doesn't have privilege to changer uid of file %s\n", running->uid, argv[0]);
    iput(chown_mip);
    return -1;
  }
  if (running->gid == new_gid || running->gid == 0) {
    chown_mip->INODE.i_gid = new_gid;
    chown_mip->dirty = 1;
  } else {
    printf("my_chown: user %d doesn't have privilege to changer gid of file %s\n", running->gid, argv[0]);
    iput(chown_mip);
    return -1;
  }
  
  iput(chown_mip);
  printf("my_chown: successfully changed owner\n");
  return 0;
}