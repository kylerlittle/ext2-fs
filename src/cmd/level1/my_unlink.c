#include "my_unlink.h"

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

/* Destroy link file (argv[0]); if error occurs, return -1 */
int my_unlink(int argc, char *argv[])
{
  /* argv[0] is the link file we want to destroy (remove from memory); this can also
  remove the file itself if link count == 0
  IDEA: 
  1. Get the inode into memory
  2. 
  */

  if (argc < 1) {
    printf("my_unlink: no link file supplied to remove\n");
    return -1;
  }

  int link_dev, parent_dev;
  u32 link_ino, parent_inode;
  MINODE *link_mip, *parent_mip;

  // Per usual, get correct device for argv[0] file
  if (argv[0][0] == '/') link_dev = root->dev;
  else link_dev = running->cwd->dev;

  // get argv[0] inode into memory; recall getino tokenizes for us
  link_ino = getino(&link_dev, argv[0]);
  if (!link_ino) {
    printf("my_unlink: cannot remove link for %s; doesn't exist\n", argv[0]);
    return -1;
  }

  // Get argv[0] minode
  link_mip = iget(link_dev, link_ino);

  // Now, make sure check argv[0] is a REG or LNK file (DIRS CAN'T BE LINKS!!!!)
  if (S_ISDIR(link_mip->INODE.i_mode)) {
    printf("my_unlink: %s is a directory; therefore, it's not a link\n", argv[0]);
    return -1;
  }

  // check directory of argv[0] exists and is a DIR
  char parent_dir[MAX_FILENAME_LEN], link_child[MAX_FILENAME_LEN];
  strcpy(parent_dir, dirname(argv[0]));
  strcpy(link_child, basename(argv[0]));
  printf("my_unlink: parent_dir is %s; child to unlink is %s\n", parent_dir, link_child);

  // I guess maybe dev could be different for parent, so get correct device to be sure
  if (argv[0][0] == '/') parent_dev = root->dev;
  else parent_dev = running->cwd->dev;
  parent_inode = getino(&parent_dev, parent_dir);  // return non-zero if exists
  if (parent_inode == 0) {
    printf("my_unlink: %s directory doesn't exist\n", parent_dir);
    return -1;
  }
  // Since exists, get that bitch's mip pointer
  parent_mip = iget(parent_dev, parent_inode);

  // If we made it here, we are safe to attempt removing the link now; first decrement link count
  link_mip->INODE.i_links_count--;
  // Remove link_child (which is basename(argv[0])) from directory INODE data block
  rmChild(parent_mip, link_child);

  // parent mip is modified, so mark as dirty and write back to disc
  parent_mip->dirty = 1;
  iput(parent_mip);
  
  if (link_mip->INODE.i_links_count > 0) {
    printf("my_unlink: link count for %s is now %d\n", link_child, link_mip->INODE.i_links_count);
    link_mip->dirty = 1;
    iput(link_mip);
  } else { // link count is now 0, time to remove this file!!
    /* Deallocate all of its data blocks. Pretty easy...
    link_mip->INODE.i_block array contains pointers to the data blocks. Simply go to address, deallocate (memset 0),
    then set the pointer to null. Kinda complicated for indirect data blocks, but whatevs */
    printf("my_unlink: link count for %s is 0, so deallocating all data blocks\n", link_child);
    // Before truncating, check 
    truncate(link_mip);
    iput(link_mip);
  }
  return 0;
}