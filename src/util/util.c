/************** util.c file ****************/
#include "type.h"
#include "util.h"

/**** globals defined in main.c file ****/
MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
char gpath[MAX_FILENAME_LEN];
char *name[MAX_COMPONENTS];
int n;
int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;
char line[256], cmd[32], pathname[256];

int get_block(int dev, int blk, char *buf)
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
  return 0;
}   

/* Return MINODE pointer to inode #ino on device dev. */
MINODE *iget(int dev, int ino)
{
  MINODE *mip;
  // // return minode pointer to loaded INODE
  // (1). Search minode[ ] for an existing entry (refCount > 0) with 
  //      the needed (dev, ino):
  //      if found: inc its refCount by 1;
  //                return pointer to this minode;

  // (2). // needed entry not in memory:
  //      find a FREE minode (refCount = 0); Let mip-> to this minode;
  //      set its refCount = 1;
  //      set its dev, ino

  // (3). load INODE of (dev, ino) into mip->INODE:
       
  //      // get INODE of ino a char buf[BLKSIZE]    
  //      blk    = (ino-1) / 8 + inode_start;
  //      offset = (ino-1) % 8;

  //      printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

  //      get_block(dev, blk, buf);
  //      ip = (INODE *)buf + offset;
  //      mip->INODE = *ip;  // copy INODE to mp->INODE

  return mip;
}


int iput(MINODE *mip) // dispose a used minode by mip
{
  mip->refCount--;
 
  // if (mip->refCount > 0) return;
  // if (!mip->dirty)       return;
 
  // Write YOUR CODE to write mip->INODE back to disk
  return 0;
} 


// serach a DIRectory INODE for entry with a given name
int search(MINODE *mip, char *name)
{
   // return ino if found; return 0 if NOT
   return 0;
}


// retrun inode number of pathname

int getino(char *pathname)
{ 
   // SAME as LAB6 program: just return the pathname's ino;
   return 0;
}



// THESE two functions are for pwd(running->cwd), which prints the absolute
// pathname of CWD. 

int findmyname(MINODE *parent, u32 myino, char *myname) 
{
   // parent -> at a DIR minode, find myname by myino
   // get name string of myino: SAME as search except by myino;
   // copy entry name (string) into myname[ ];
   return 0;
}


int findino(MINODE *mip, u32 *myino)
{
  // fill myino with ino of . 
  // retrun ino of ..
  return 0;
}
