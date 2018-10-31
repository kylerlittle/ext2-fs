#include "constants.h"
#include "tokenize.h"
#include <ext2fs/ext2_fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int fd;
char *name[64];

INODE *ip;
char dbuf[BLKSIZE], temp[256];
DIR *dp;
char *cp;
int i;
int dev;
char *disk = "../mydisk", *pathname = "dir1";

int show_block() {
    int n = tokenize(name, pathname, "/");
    // ip = inone structure of directory
    dev = fd;

    int ino, blk, offset;
    int iblk; //= inodes_start_block number (YOU should also know HOW)
    char ibuf[BLKSIZE];

    for (i=0; i < n; i++){
        ino = search(ip, name[i]);
        if (ino==0){
        printf("can't find %s\n", name[i]); exit(1);
    }

        // Mailman's algorithm: Convert (dev, ino) to inode pointer
        blk    = (ino - 1) / 8 + iblk;  // disk block contain this INODE 
        offset = (ino - 1) % 8;         // offset of INODE in this block
        get_block(dev, blk, ibuf);
        ip = (INODE *)ibuf + offset;    // ip -> new INODE
    }

    clear_tok_list(name);
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        disk = argv[1];
        pathname = argv[2];
    }

    fd = open(disk, O_RDONLY);
    if (fd < 0){
        printf("open %s failed\n", disk);
        exit(1);
    }
    show_block();
}