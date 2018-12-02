#include "my_symlink.h"
#include "my_creat.h"

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
int my_symlink(int argc, char *argv[])
{
  /* argv[0] is oldFileName; argv[1] is the new link to argv[0]
  IDEA: create new file which has same inode # as that of argv[0];
  then increment argv[0]'s inode's link_count. do errors checks as well. */

  if (argc < 2) {
    printf("my_symlink: not enough args supplied\n");
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
    printf("my_symlink: cannot make link for %s; doesn't exist\n", argv[0]);
    return -1;
  }

  // get correct device for new link we're creating (wanna make sure we putting link on correct dev)
  if (argv[1][0] == '/') new_dev = root->dev;
  else new_dev = running->cwd->dev;

  // MAKE SURE LINK DOESN'T ALREADY EXIST!
  new_inode = getino(&new_dev, argv[1]);
  if (new_inode) { // non-zero returns means it exists already
    printf("my_symlink: %s already exists; can't creating link\n", argv[1]);
    return -1;
  }

  // Get argv[0] minode
  old_mip = iget(old_dev, old_inode);

  // check argv[0] is a REG or LNK file (link to DIR is NOT allowed).
  if (S_ISDIR(old_mip->INODE.i_mode)) {
    printf("my_symlink: %s is a directory; can't link to dir\n", argv[0]);
    return -1;
  }

  // check directory of argv[1] exists and is a DIR but that file does not yet exist in that directory (already did latter above)
  char parent_dir[MAX_FILENAME_LEN], link_child[MAX_FILENAME_LEN];
  strcpy(parent_dir, dirname(argv[1]));
  strcpy(link_child, basename(argv[1]));

  parent_inode = getino(&new_dev, parent_dir);  // return non-zero if exists
  if (parent_inode == 0) {
    printf("my_symlink: %s directory doesn't exist\n", parent_dir);
    return -1;
  }

  // Add an entry [ino rec_len name_len argv[1]] to the data block of parent_dir
  // This creates argv[1], which has the SAME ino as that of argv[0]; note both must be on same dev!
  parent_mip = iget(new_dev, parent_inode);
  
  // Create a new file under the parent minode pointer.
  creat_child_under_pmip(parent_mip, link_child);
  parent_mip->dirty = 1;  // dirty now
  iput(parent_mip);

  // Now, we should be able to get the inode number of the symlink
  new_inode = getino(&new_dev, argv[1]);
  if (new_inode == 0) {
    printf("my_symlink: creating link failed due to creat\n");
    return -1;
  }
  new_mip = iget(new_dev, new_inode);
  // Set mode to SYMLINK to LNK with rwx for all three things (user, group, other)
  new_mip->INODE.i_mode = 0120777; 
  new_mip->dirty = 1;
  // Copy name of argv[0] (i.e. file we're trying to make link for) into i_block of new inode
  // since i_block is array of size 15 ints, it can maximally store 60 chars... so just copy first 60 chars of argv[0]
  strncpy((char *)new_mip->INODE.i_block, argv[0], 60);
  new_mip->INODE.i_size = strlen(argv[0]);
  iput(new_mip);
  
  // Write back to disc baby!!
  printf("my_symlink: successful creation of link; %s -> %s\n", argv[1], argv[0]);
}