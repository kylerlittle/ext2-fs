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
char line[MAX_INPUT_LEN], cmd[32], pathname[MAX_FILENAME_LEN];

int tokenize(char *tok_list[], char *input_line, char *delimiters)
{
    char *s;
    int n = 0;
    
    /* Clear tok_list */
    clear_tok_list(tok_list);

    strcpy(gpath, input_line);  // make copy of input_line; work with this
    s = strtok(gpath, delimiters);
    while (s) {
        tok_list[n] = (char*)malloc(strlen(s) + 1);
        strcpy(tok_list[n++], s);
        s = strtok(NULL, delimiters);
    }
    /* Mark the end of the tok list */
    tok_list[n] = NULL;

    /* Debug info */
    int i=0;
    printf("tokenized: ");
    while (tok_list[i]) printf("%s ", tok_list[i++]);
    printf("\n");

    return n;
}

int clear_tok_list(char *tok_list[]) {
    clear_toks_after_i(tok_list, 0);    
}

int clear_toks_after_i(char *tok_list[], int i) {
    while (tok_list[i]) {
        free(tok_list[i]);
        tok_list[i++] = NULL;
    }
}

int get_block(int dev, int blk, char *buf)
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk * BLKSIZE, 0);
  write(dev, buf, BLKSIZE);
}   

/* Return MINODE pointer to inode #ino on device dev. */
MINODE *iget(int dev, int ino)
{
  int i, blk, disp;
  MINODE *mip = NULL;
  char buf[BLKSIZE];
  INODE *ip;
  
  // 1. Search minode[] for an existing entry (refCount > 0) with 
  //    the needed (dev, ino); return pointer to this minode;  
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino && mip->refCount > 0)
    {
      mip->refCount++;  // if found: inc its refCount by 1;
      if (DEBUG_MODE) printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
      return mip;
    }
  }

  // 2. needed entry not in memory:
  //      find a FREE minode (refCount = 0); Let mip-> to this minode;
  //      set its refCount = 1;
  //      set its dev, ino
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount == 0)
    {
      if (DEBUG_MODE) printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
      mip->refCount = 1;
      mip->dev = dev;
      mip->ino = ino;
      mip->dirty = 0;
      mip->mounted = 0;
      mip->mptr = NULL;
      // 3. Lastly, load INODE of (dev, ino) into mip->INODE:
      // printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

      blk = (ino - 1) / 8 + inode_start;
      disp = (ino - 1) % 8;

      get_block(dev, blk, buf);  // get INODE of ino a char buf[BLKSIZE]    
      ip = (INODE *)buf + disp;
      
      mip->INODE = *ip;  // copy INODE to mp->INODE
      return mip;
    }
  }
  printf("PANIC: no more free minodes\n");
  return mip;
}


void iput(MINODE *mip) // dispose a used minode by mip
{
  int i, block, offset;
  char buf[BLKSIZE];
  INODE *ip;

  if (mip == 0)
    return;

  mip->refCount--;

  // if refs still exist, don't write back yet since unable to sync
  if (mip->refCount > 0)
    return;
  // if not dirty, no need to write back
  if (!mip->dirty)
    return;

  /* write back */
  printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);

  block = ((mip->ino - 1) / 8) + inode_start;
  offset = (mip->ino - 1) % 8;

  /* first get the block containing this inode */
  get_block(mip->dev, block, buf);

  ip = (INODE *)buf + offset;
  *ip = mip->INODE;

  put_block(mip->dev, block, buf);
} 


// serach a DIRectory INODE for entry with a given name
int search(MINODE *mip, char *name)
{
  int i;
  char *cp, c, sbuf[BLKSIZE];
  DIR *dp;
  INODE *ip;

  printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
  ip = &(mip->INODE);

  /**********  search for a file name ***************/
  for (i = 0; i < 12; i++)
  { /* search direct blocks only */
    if (ip->i_block[i] == 0)
      return 0;
    printf("search: i=%d  i_block[%d]=%d\n", i, i, ip->i_block[i]);

    get_block(dev, ip->i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    printf("   i_number rec_len name_len    name\n");

    while (cp < sbuf + BLKSIZE)
    {
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = 0;

      printf("%8d%8d%8u        %s\n",
             dp->inode, dp->rec_len, dp->name_len, dp->name);
      if (strcmp(dp->name, name) == 0)
      {
        printf("found %s : ino = %d\n", name, dp->inode);
        return (dp->inode);
      }
      dp->name[dp->name_len] = c;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}


/* Return the inode number of pathname on device dev. Return 0 if doesn't exist. */
int getino(int *dev, char *pathname)
{ 
  // SAME as LAB6 program: just return the pathname's ino;
  int i, ino, blk, disp;
  // char temp[MAX_FILENAME_LEN];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s dev=%d\n", pathname, *dev);
  if (strcmp(pathname, "/") == 0)
    return 2;

  if (pathname[0] == '/')
    mip = iget(*dev, 2);
  else
    mip = iget(running->cwd->dev, running->cwd->ino);

  // strcpy(temp, pathname);
  n = tokenize(name, pathname, "/");

  for (i = 0; i < n; i++)
  {
    printf("===========================================\n");
    printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);

    ino = search(mip, name[i]);

    if (ino == 0)
    {
      iput(mip);
      printf("name %s does not exist\n", name[i]);
      return 0;
    }
    iput(mip);
    mip = iget(*dev, ino);
  }
  clear_tok_list(name);  // clean up that malloc shit
  return ino;
}



// THESE two functions are for pwd(running->cwd), which prints the absolute
// pathname of CWD. 

int findmyname(MINODE *parent, u32 myino, char *myname) 
{
  // parent -> at a DIR minode, find myname by myino
  // get name string of myino: SAME as search except by myino;
  // copy entry name (string) into myname[ ];

  int i;
  char buf[BLKSIZE], temp[256], *cp;
  DIR *dp;
  MINODE *mip = parent;

  /**********  search for a file name ***************/
  for (i = 0; i < 12; i++)
  { /* search direct blocks only */
    if (mip->INODE.i_block[i] == 0)
      return -1;

    get_block(mip->dev, mip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE)
    {
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      //printf("%s  ", temp);

      if (dp->inode == myino)
      {
        strncpy(myname, dp->name, dp->name_len);
        myname[dp->name_len] = 0;
        return 0;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return (-1);
}


int findino(MINODE *mip, u32 *myino)
{
  // fill myino with ino of . 
  // retrun ino of ..
  char buf[BLKSIZE], *cp;
  DIR *dp;

  get_block(mip->dev, mip->INODE.i_block[0], buf);
  cp = buf;
  dp = (DIR *)buf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  return dp->inode;
}

//refer to page 334 for enter name function in system programming (4(4))
int enter_name(MINODE *minodePtr, int ino, char *name)
{
    int i=0;
    INODE *parent_inodePtr;
    parent_inodePtr = &minodePtr->INODE;
    char buf[BLKSIZE]; 
    char *cp;
    DIR *dp;
    int bno = 0;
    int need_length = 0;
    int ideal_length = 0;
    int remain;
    //can assume only 12 direct blocks
    for (; i < parent_inodePtr->i_size / BLKSIZE; i++)
    {
        if (parent_inodePtr->i_block[i] == 0) //page 334
        {
            break;
        }
        bno = parent_inodePtr->i_block[i];
        get_block(dev, bno, buf);
        dp = (DIR *)buf;
        cp = buf;

        //need length from page 335
        //all dir entries rec_len=ideal length except the last entry
        //the rec_length of the last entry may be longer than the ideal length

        need_length = 4 * ((8 + strlen(name) + 3) / 4); //->proves its a multiple of 4
        printf("need_length: %d\n", need_length);
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len; //rec_length is 12?? last block
            dp = (DIR *)cp;
        }
        //now we are at the last block
        printf("last_entry: %s\n", dp->name);
        cp = (char *)dp;

        //we handle this later, the last entry may not be the ideal length, we will trim it below
        ideal_length = 4 * ((8 + dp->name_len + 3) / 4);

        //now whats remaining is remain which is the last entry rec_len-ideal_length
        remain = dp->rec_len - ideal_length;
        printf("remain: %d\n", remain);
        if (remain >= need_length)
        {
            //we need to trim it to its ideal length
            dp->rec_len = ideal_length;
            cp += dp->rec_len;
            dp = (DIR *)cp;

            dp->inode = ino;
            dp->rec_len = 1024 - ((u32)cp - (u32)buf);
            printf("rec_len: %d\n", dp->rec_len);
            dp->name_len = strlen(name);
            dp->file_type = EXT2_FT_DIR; //this is 2, and indicates directory file
            strcpy(dp->name, name);
            //writes
            put_block(dev, bno, buf);
            return 1;
        }
    }

    printf("Number of Data Blocks: %d\n", i);
    
    dp->inode=ino;
    dp->rec_len=BLKSIZE; //again BLKSIZE is 1024 macro defined
    dp->name_len=strlen(name);
    dp->file_type=EXT2_FT_DIR; //this is 2, and indicates directory file
    strcpy(dp->name, name);

    //now we write back, but we have updated all variables
    put_block(dev, bno, buf);
    return 1;
}

//page 338
void rm_child(MINODE *parent_minodePtr, char *name)
{
    int i;
    char buf[BLKSIZE];
    char *cp;
    char *final_cp;
    int first;
    int last;
    char temp[64];
    INODE *parent_inodePtr = &parent_minodePtr->INODE;
    DIR *dp;
    DIR *predecessor_dp;
    DIR *start_dp; //for when we have to find the child, first or middle

    printf("Attempting to remove %s\n", name);

    for (i = 0; i < 12; i++)
    {
        if (parent_inodePtr->i_block[i] == 0)
        {
            return 0;
        }
        get_block(dev, parent_inodePtr->i_block[i], buf);
        cp = buf;
        dp = (DIR *)buf;

        while (cp < buf + BLKSIZE) //traversal of entries
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            if (!strcmp(temp, name)) //found and first
            {
                printf("Child found\n");
                if (cp == buf && cp + dp->rec_len == buf + BLKSIZE)
                {
                    free(buf);
                    bdealloc(dev, ip->i_block[i]); //we are now deallocating the block

                    parent_inodePtr->i_size -= BLKSIZE;
                    //shift left parents i_block page 340
                    while (parent_inodePtr->i_block[i + 1] && i + 1 < 12)
                    {
                        i++;
                        get_block(dev, parent_inodePtr->i_block[i], buf);
                        put_block(dev, parent_inodePtr->i_block[i - 1], buf);
                    }
                }
                else if (cp + dp->rec_len == buf + BLKSIZE) //last entry in block
                {
                    predecessor_dp->rec_len += dp->rec_len;
                    put_block(dev, parent_inodePtr->i_block[i], buf);
                }
                else //the child is either the first or its in the middle
                {    //but... theres stuff after child
                    final_cp = buf;
                    start_dp = (DIR *)buf;

                    while (final_cp + start_dp->rec_len < buf + BLKSIZE)
                    {
                        final_cp += start_dp->rec_len; //add deleted rec_len to the last entry
                        start_dp = (DIR *)final_cp;
                    }

                    start_dp->rec_len += dp->rec_len;

                    first = cp + dp->rec_len; //add deleted rec_len to the last entry
                    last = buf + BLKSIZE;
                    memcpy(cp, first, last - first); //move all trailing entries left page 340
                    put_block(dev, parent_inodePtr->i_block[i], buf);
                }
                parent_minodePtr->dirty = 1;
                iput(parent_minodePtr); //releases a used minode pointed to by ptr
                return;
            }
            predecessor_dp = dp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    return;
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

/* Deallocate inode on device. */
int idealloc(int dev, int ino) {
  char buf[BLKSIZE];
  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);
  // write buf back
  put_block(dev, imap, buf);
  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int decFreeDataBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int incFreeDataBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

/* Allocate data block on dev. In other words, on device dev,
write to datablock bitmap that we added a data block. */
int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read datablock_bitmap block
  get_block(dev, bmap, buf);

  for (i=0; i < nblocks; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeDataBlocks(dev);

       put_block(dev, bmap, buf);

       return i+1;
    }
  }
  printf("balloc(): no more free blocks\n");
  return 0;
}

/* Deallocate data block number ino on dev. */
int bdealloc(int dev, int ino) {
  char buf[BLKSIZE];
  // get inode bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, ino-1);
  // write buf back
  put_block(dev, bmap, buf);
  // update free inode count in SUPER and GD
  incFreeDataBlocks(dev);
}

/* Remove all of mip->INODE's data blocks! Then iput back to disc. */
int truncate(MINODE *mip) {
  printf("truncate: deallocating direct block numbers\n");
  int i;
  char buf[BLKSIZE], indirect_buf[BLKSIZE];
  for (i=0; i < 15; ++i) {
      if (ip->i_block[i] == 0) break;
      printf("i_block[%d] = %d\n", i, ip->i_block[i]);
      // remove direct block now
      bdealloc(mip->dev, mip->INODE.i_block[i]);
      ip->i_block[i] = 0;
  }
  if (i >= 12) {
      printf("truncate: deallocating indirect block numbers\n");
      get_block(dev, ip->i_block[12], buf);
      int * int_p = (int *)buf, counter = 0;
      while (counter < BLKSIZE / sizeof(int)) {
          if (*int_p == 0) break;
          printf("%d  ", *int_p);
          // remove indirect block now
          bdealloc(mip->dev, *int_p);
          *int_p = 0;  // set it to 0 now since it's been deallocated
          int_p++; counter++;
      }
  }
  bdealloc(mip->dev, mip->INODE.i_block[12]);
  ip->i_block[12] = 0; // clear this
  if (i >= 13) {
      printf("truncate: deallocating double indirect block numbers\n");
      get_block(dev, ip->i_block[13], buf);
      int * int_p = (int *)buf, counter = 0;
      while (counter < BLKSIZE / sizeof(int)) {
          if (*int_p == 0) break;
          get_block(dev, *int_p, indirect_buf);
          int * double_int_p = (int *)indirect_buf, i_counter = 0;
          while (i_counter < BLKSIZE / sizeof(int)) {
              if (*double_int_p == 0) break;
              printf("%d  ", *double_int_p);
              // remove double indirect block now
              bdealloc(mip->dev, *double_int_p);
              *double_int_p = 0;  // set it to 0 now since it's been deallocated
              double_int_p++; i_counter++;
          }
          int_p++; counter++;
      }
      printf("\n");
  }
  bdealloc(mip->dev, mip->INODE.i_block[13]);
  ip->i_block[13] = 0; // clear this
}