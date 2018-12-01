#include "my_readlink.h"

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

/* Read link argv[0]; if error occurs, return -1 */
int my_readlink(int argc, char *argv[])
{
  /* argv[0] is oldFileName; argv[1] is the new link to argv[0]
  IDEA: create new file which has same inode # as that of argv[0];
  then increment argv[0]'s inode's link_count. do errors checks as well. */

  if (argc < 2) {
    printf("my_link: not enough args supplied\n");
    return -1;
  }

  int old_dev, new_dev;
  u32 old_inode, new_inode, parent_inode;
  MINODE *old_mip, *new_mip, *parent_mip;

  // get correct device for argv[0] file
  if (argv[0][0] == '/') old_dev = root->dev;
  else old_dev = running->cwd->dev;

  // get argv[0] inode into memory (since we need its ino and to update it); recall getino tokenizes for us
  old_inode = getino(&old_dev, argv[0]);
  if (!old_inode) {
    printf("my_link: cannot make link for %s; doesn't exist\n", argv[0]);
    return -1;
  }

  // get correct device for new link we're creating (wanna make sure we putting link on correct dev)
  if (argv[1][0] == '/') new_dev = root->dev;
  else new_dev = running->cwd->dev;

  // MAKE SURE LINK DOESN'T ALREADY EXIST!
  new_inode = getino(&new_dev, argv[1]);
  if (new_inode) { // non-zero returns means it exists already
    printf("my_link: %s already exists; can't creating link\n", argv[1]);
    return -1;
  }

  // Get argv[0] minode
  old_mip = iget(old_dev, old_inode);

  // check argv[0] is a REG or LNK file (link to DIR is NOT allowed).
  if (S_ISDIR(old_mip->INODE.i_mode)) {
    printf("my_link: %s is a directory; can't link to dir\n", argv[0]);
    return -1;
  }
}