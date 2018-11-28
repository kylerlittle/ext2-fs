#include "my_ls.h"

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

int ls_file(MINODE *mip, char *name)
{
  int k;
  u16 mode, mask;
  char mydate[32], *s, *cp, ss[32];
  mode = mip->INODE.i_mode;
  if (S_ISDIR(mode))
    putchar('d');
  else if (S_ISLNK(mode))
    putchar('l');
  else
    putchar('-');

  mask = 000400;
  for (k = 0; k < 3; k++)
  {
    if (mode & mask)
      putchar('r');
    else
      putchar('-');
    mask = mask >> 1;

    if (mode & mask)
      putchar('w');
    else
      putchar('-');
    mask = mask >> 1;

    if (mode & mask)
      putchar('x');
    else
      putchar('-');
    mask = mask >> 1;
  }
  printf("%4d", mip->INODE.i_links_count);
  printf("%4d", mip->INODE.i_uid);
  printf("%4d", mip->INODE.i_gid);
  printf("  ");
  // s = mydate;
  // s = (char *)ctime(&mip->INODE.i_ctime);
  // s = s + 4;
  // strncpy(ss, s, 12);
  // ss[12] = 0;

  // printf("%s", ss);
  printf("%8ld", mip->INODE.i_size);

  printf("    %s", name);

  if (S_ISLNK(mode))
    printf(" -> %s", (char *)mip->INODE.i_block);
  printf("\n");
}

int ls_dir(MINODE *mip)
{
  int i;
  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  MINODE *dip;

  for (i = 0; i < 12; i++)
  { /* search direct blocks only */
    printf("i_block[%d] = %d\n", i, mip->INODE.i_block[i]);
    if (mip->INODE.i_block[i] == 0)
      return 0;

    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    // printf("   i_number rec_len name_len   name\n");

    while (cp < sbuf + BLKSIZE)
    {
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      //printf("%8d%8d%8u        %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

      dip = iget(dev, dp->inode);
      ls_file(dip, temp);
      iput(dip);

      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
}

int my_ls(int argc, char *argv[]) {
    MINODE *mip;
    u16 mode;
    int dev, ino;

    if (argv[0] == NULL) {
        ls_dir(running->cwd);
    }
    else
    {
        dev = root->dev;
        ino = getino(&dev, argv[0]);
        if (ino == 0)
        {
            printf("no such file %s\n", argv[0]);
            return -1;
        }
        mip = iget(dev, ino);
        mode = mip->INODE.i_mode;
        if (!S_ISDIR(mode))
            ls_file(mip, (char *)basename(argv[0]));
        else
            ls_dir(mip);
        iput(mip);
    }
}