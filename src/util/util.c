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
char line[MAX_INPUT_LEN], cmd[32], pathname[256];

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
    if (mip->dev == dev && mip->ino == ino)
    {
      mip->refCount++;  // if found: inc its refCount by 1;
      //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
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
      printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
      mip->refCount = 1;
      mip->dev = dev;
      mip->ino = ino;
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

  if (mip->refCount > 0)
    return;
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
    //getchar();

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
  return (0);
}


// retrun inode number of pathname

int getino(char *pathname)
{ 
  // SAME as LAB6 program: just return the pathname's ino;
  int i, ino, blk, disp;
  // char temp[MAX_FILENAME_LEN];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/") == 0)
    return 2;

  if (pathname[0] == '/')
    mip = iget(dev, 2);
  else
    mip = iget(running->cwd->dev, running->cwd->ino);

  // strcpy(temp, pathname);
  tokenize(name, pathname, "/");

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
    mip = iget(dev, ino);
  }
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
