#include "my_stat.h"

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

int stat_file(MINODE *mip, char *name)
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
  char filetime[256];
  strcpy(filetime, ctime(&mip->INODE.i_mtime));
  filetime[strlen(filetime)-1] = 0; // kill '\n'
  printf("  %s", filetime);

  printf("    %s", name);

  if (S_ISLNK(mode))
    printf(" -> %s", (char *)mip->INODE.i_block);
  printf("\n");
  int j = 0;
  for (j=0; j<15; j++) {
    if (mip->INODE.i_block[j] == 0) {
      printf("Num Blocks: %d\n", j);
      break;
    }
  }
  printf("Num Links: %d\n", mip->INODE.i_links_count);
  char buf[BLKSIZE];
  get_block(dev, mip->INODE.i_block[1], buf);
  cp = buf;
  dp = (DIR *)buf;
  while (cp < buf + BLKSIZE)
  {
      strncpy(name, dp->name, dp->name_len); //for n in strncpy
      name[dp->name_len] = 0;

      if (name[0]==0) break;
      printf("name=%s\n", name);
      cp += dp->rec_len;
      dp = (DIR *)cp;
  }
}

int my_stat(int argc, char *argv[]) {
    MINODE *mip;
    u16 mode;
    int dev, ino;

    dev = root->dev;
    ino = getino(&dev, argv[0]);
    if (ino == 0)
    {
        printf("no such file %s\n", argv[0]);
        return -1;
    }
    mip = iget(dev, ino);
    mode = mip->INODE.i_mode;
    stat_file(mip, (char *)basename(argv[0]));
    iput(mip);
}