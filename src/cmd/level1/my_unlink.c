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
  /* argv[0] is the link file we want to destroy (remove from memory)
  IDEA: 
  1. Get the inode
  */

  if (argc < 1) {
    printf("my_unlink: no link file supplied to remove\n");
    return -1;
  }

  int link_dev;
  u32 link_ino, parent_inode;
  MINODE *link_mip, *parent_mip;

  // Per usual, get correct device for argv[0] file
  if (argv[0][0] == '/') link_dev = root->dev;
  else link_dev = running->cwd->dev;

  // get argv[0] inode into memory (); recall getino tokenizes for us
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

  // check directory of argv[1] exists and is a DIR but that file does not yet exist in that directory (already did latter above)
  char parent_dir[MAX_FILENAME_LEN], link_child[MAX_FILENAME_LEN];
  strcpy(parent_dir, dirname(argv[1]));
  strcpy(link_child, basename(argv[1]));

  parent_inode = getino(&new_dev, parent_dir);  // return non-zero if exists
  if (parent_inode == 0) {
    printf("my_link: %s directory doesn't exist\n", parent_dir);
    return -1;
  }

  // If we made it here, we are safe to attempt removing the link now; first decrement link count
  link_mip->INODE.i_links_count--;

  // If link count is now 0, time to remove this file!!
  
  if (link_mip->INODE.i_links_count == 0) {
    /* Deallocate all of its data blocks. Pretty easy...
    link_mip->INODE.i_block array contains pointers to the data blocks. Simply go to address, deallocate (memset 0),
    then set the pointer to null. Kinda complicated for indirect data blocks, but whatevs */

  } 



  // Add an entry [ino rec_len name_len argv[1]] to the data block of parent_dir
  // This creates argv[1], which has the SAME ino as that of argv[0]; note both must be on same dev!
  parent_mip = iget(new_dev, parent_inode);
  // append entry to parent_mip's data block; store result in int.
  
  int result = enter_name(parent_mip, link_mip->ino, link_child);

  // Increment link count; mark as dirty; write back
  link_mip->INODE.i_links_count++;
  link_mip->dirty = 1;
  printf("my_link: link count for %s is now %d\n", argv[0], link_mip->INODE.i_links_count);
  
  iput(link_mip);
  iput(parent_mip);

  return result;
}