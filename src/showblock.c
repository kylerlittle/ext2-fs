/********* showblock.c: locates the file named PATHNAME under '/' directory
 *         and prints the disk blocks (direct, 
 *         indirect, double-indirect) of the file.
 *********/

#include "constants.h"
#include "tokenize.h"
#include <ext2fs/ext2_fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int fd;
char *name[64];

INODE *ip;
DIR *dp;
GD *gp;
SUPER *sp;

char *cp;
int i;
int dev;
char *disk = "../mydisk", *pathname = "dir1";

int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd,(long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int search(INODE *ip, char *name) {
    int i = 0;
    char dbuf[BLKSIZE], *cp, temp[MAX_FILENAME_LEN];

    for (i = 0; i < NUM_DIRECT_BLKS; i++) {
        if (ip->i_block[i] == 0) return 0;   // NOT FOUND

        // Otherwise, search for name in dir; read direct block into dbuf
        get_block(fd, ip->i_block[i], dbuf);

        dp = (DIR *)dbuf;
        cp = dbuf;

        // Print stuff like KC wang does to check our work.
        printf("searching for %s in MINODE = [dev : %d, ino : %d]\n", name, dev, dp->inode);
        printf(" ino     rec_len   name_len   name\n");

        while (cp < dbuf + BLKSIZE) {
            // First, print
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            printf("%4d   %6d     %6d      %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

            // Now, if name matches with dir's name, return inode number
            if (!strncmp(dp->name, name, dp->name_len)) {
                return dp->inode;
            }

            // Otherwise, add the record length to our pointer, recast as DIR*, and continue
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
}

int show_block() {
    int ino, blk, offset;
    char ibuf[BLKSIZE], buf[BLKSIZE];

    // STEP 1: Check if EXT2 File system!!!
        // Read SUPER block
    get_block(fd, 1, buf);  
    sp = (SUPER *)buf;

        // Check for EXT2 magic number:
    printf("s_magic = %x\n", sp->s_magic);
    if (sp->s_magic != 0xEF53) {
        printf("NOT an EXT2 FS\n");
        exit(1);
    }
    printf("EXT2 FS OK\n");

    // STEP 2: Get a pointer to the start of the inodes.
        // Set device to file descriptor of opened disk image (for later)
    dev = fd;

        // Next, read GD. This lets us determines where the inodes start.
    get_block(fd, 2, buf);
    gp = (GD *)buf;

        // Print bmap, imap, and inode table block numbers.
    printf("bmap = %d\t", gp->bg_block_bitmap);
    printf("imap = %d\t", gp->bg_inode_bitmap);
    printf("iblock = %d\n", gp->bg_inode_table);
    
        // Now, get inode start block number
    int iblk = gp->bg_inode_table;

        // Next, get inode start block. Load into buf.
    get_block(fd, iblk, buf);

        // Set ip to point at 2nd INODE (root inode)
    ip = (INODE *)buf + 1;

    // STEP 3: Tokenize pathname.
    char cpy[MAX_FILENAME_LEN];
    strcpy(cpy, pathname);
    int n = tokenize(name, cpy, "/"), i = 0;
    printf("tokenizing %s\n", pathname);
    while (name[i]) {
        printf("%s  ", name[i++]);
    }
    printf("\n");

    // STEP 4: Get ip to point at inode of pathname!
    for (i=0; i < n; i++){
        printf("=================================================\n");
        printf("getino: i=%d   name[%d]=%s\n", i, i, name[i]);
        
        ino = search(ip, name[i]);
        if (ino == 0) {
            printf("can't find %s\n", name[i]);
            exit(1);
        }
        printf("found %s : ino = %d\n", name[i], ino);

        // Mailman's algorithm: Convert (dev, ino) to inode pointer
        blk    = (ino - 1) / 8 + iblk;  // disk block contain this INODE 
        offset = (ino - 1) % 8;         // offset of INODE in this block
        get_block(dev, blk, ibuf);
        ip = (INODE *)ibuf + offset;    // ip -> new INODE
    }
    // STEP 5: Print direct, indirect, and double indirect block numbers.
        // Note that ip now points at INODE of pathname
    printf("DIRECT BLOCK NUMBERS:\n");
    for (i=0; i < 15; ++i) {
        if (ip->i_block[i] == 0) break;
        printf("i_block[%d] = %d\n", i, ip->i_block[i]);
    }
    if (i >= 12) {
        printf("INDIRECT BLOCK NUMBERS:\n");
        get_block(dev, ip->i_block[12], buf);
        int * int_p = (int *)buf, counter = 0;
        while (counter < BLKSIZE / sizeof(int)) {
            if (*int_p == 0) break;
            printf("%d  ", *int_p);
            int_p++; counter++;
        }
    }
    if (i >= 13) {
        printf("\nDOUBLE INDIRECT BLOCK NUMBERS:\n");
        get_block(dev, ip->i_block[13], buf);
        int * int_p = (int *)buf, counter = 0;
        while (counter < BLKSIZE / sizeof(int)) {
            if (*int_p == 0) break;
            get_block(dev, *int_p, ibuf);
            int * double_int_p = (int *)ibuf, i_counter = 0;
            while (i_counter < BLKSIZE / sizeof(int)) {
                if (*double_int_p == 0) break;
                printf("%d  ", *double_int_p);
                double_int_p++; i_counter++;
            }
            int_p++; counter++;
        }
        printf("\n");
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