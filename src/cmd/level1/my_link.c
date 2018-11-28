#include "my_link.h"

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

/* Create link argv[1] for file argv[0]; if error occurs, return -1 */
int my_link(int argc, char *argv[])
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

  // check directory of argv[1] exists and is a DIR but that file does not yet exist in that directory (already did latter above)
  char parent_dir[MAX_FILENAME_LEN], link_child[MAX_FILENAME_LEN];
  strcpy(parent_dir, dirname(argv[1]));
  strcpy(link_child, basename(argv[1]));

  parent_inode = getino(&new_dev, parent_dir);  // return non-zero if exists
  if (parent_inode == 0) {
    printf("my_link: %s directory doesn't exist\n", parent_dir);
    return -1;
  }

  // Add an entry [ino rec_len name_len argv[1]] to the data block of parent_dir
  // This creates argv[1], which has the SAME ino as that of argv[0]; note both must be on same dev!
  parent_mip = iget(new_dev, parent_inode);
  // append entry to parent_mip's data block; store result in int.
  int result;

  // Increment link count; mark as dirty; write back
  old_mip->INODE.i_links_count++;
  old_mip->dirty = 1;
  printf("my_link: link count for %s is now %d\n", argv[0], old_mip->INODE.i_links_count);
  
  iput(old_mip);
  iput(parent_mip);

  return result;
}