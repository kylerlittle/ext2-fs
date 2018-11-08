/********* dir.c: print all entries under '/' directory *********/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include "constants.h"

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

int fd;
int iblock;

int search(INODE *ip, char *name) {
    int i = 0;
    char dbuf[BLKSIZE], *cp;

    for (i = 0; i < NUM_DIRECT_BLKS; i++) {
        if (ip->i_block[i] == 0) return 0;   // NOT FOUND

        // Otherwise, search for name in dir; read direct block into dbuf
        get_block(fd, ip->i_block[i], dbuf);

        dp = (DIR *)dbuf;
        cp = dbuf;

        while (cp < dbuf + BLKSIZE) {
            // if name matches with dir's name, return inode number
            if (!strncmp(dp->name, name, dp->name_len)) {
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
}

int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd,(long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int dir()
{
    char buf[BLKSIZE], dbuf[BLKSIZE], *cp, temp[MAX_FILENAME_LEN];

    // read GD
    get_block(fd, 2, buf);
    gp = (GD *)buf;

    iblock = gp->bg_inode_table;   // get inode start block#
    printf("inode_block = %d\n", iblock);

    // get inode start block     
    get_block(fd, iblock, buf);

    // get root inode #2
    ip = (INODE *)buf + 1;         // ip points at 2nd INODE

    // int ino = search(ip, "dir2");
    // if (ino) printf("ino: %d\n", ino);

    // ip = (INODE *)buf + 1;         // ip points at 2nd INODE

    int i;
    for (i = 0; i<NUM_DIRECT_BLKS; i++) {
        if (ip->i_block[i] == 0) {
            break;
        }
        // Note: ip->i_block[0-11] will yield a pointer to a direct block
        printf("i_block[%d] = %d\n", i, ip->i_block[i]);

        // Read direct block into dbuf
        get_block(fd, ip->i_block[i], dbuf);
        printf(" ino     rec_len   name_len   name\n");

        dp = (DIR *)dbuf;
        cp = dbuf;

        while (cp < dbuf + BLKSIZE) {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            
            printf("%4d   %6d     %6d      %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
}

char *disk = "../mydisk";
int main(int argc, char *argv[])
{ 
    if (argc > 1)
        disk = argv[1];

    fd = open(disk, O_RDONLY);
    if (fd < 0){
        printf("open %s failed\n", disk);
        exit(1);
    }

    dir();
}