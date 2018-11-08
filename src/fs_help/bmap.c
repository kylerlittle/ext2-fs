#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>


typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR; 

#define BLKSIZE 1024

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

char buf[BLKSIZE];
int fd;

int get_block(int fd, int blk, char buf[])
{
    lseek(fd, (long)blk*BLKSIZE,0);
    read(fd, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int bmap()
{
    int bmap, i, nblocks;
    get_block(fd,1,buf);
    sp=(SUPER *)buf;

    nblocks=sp->s_blocks_count;
    printf("nblocks = %d\n", nblocks);

    get_block(fd,2,buf);
    gp=(GD *)buf;

    bmap=gp->bg_block_bitmap;
    printf("bmap = %d\n", bmap);

    get_block(fd,bmap,buf);

    for (i=0; i < nblocks; i++){
    putchar(tst_bit(buf,i)+48);
    if(i && (i%8)==0)
        putchar(' ');
    if(i && (i%32) ==0)
        putchar('\n');
  }
  putchar('\n');
}

char *disk = "mydisk";

int main(int argc, char *argv[ ])
{
  if (argc > 1)
    disk = argv[1];

  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  bmap();
}