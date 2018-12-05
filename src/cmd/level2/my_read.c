#include "my_read.h"
//refer to page 346 in system programming book

/**** globals defined in main.c file ****/
MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;
char gpath[MAX_FILENAME_LEN];
char *name[MAX_COMPONENTS];
int n;
int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;
char line[MAX_INPUT_LEN], cmd[32], pathname[MAX_FILENAME_LEN];

int my_read(int argc, char *argv[])
{
    // 1. ask for a fd and number of bytes to read
    int fd = 0;
    int nbytes = 0;
    char fd_str[MAX_FILENAME_LEN] = {0}, nBytesStr[BLKSIZE] = {0}, buf[BLKSIZE] = {0};
    if (argc < 1)
    { // they supplied no args
        printf("my_read: enter opened file descriptor to read from : ");
        fgets(fd_str, MAX_FILENAME_LEN, stdin);
        fd_str[strlen(fd_str) - 1] = '\0';
        fd = atoi(fd_str);
        printf("my_read: enter number of bytes to read from file: ");
        fgets(nBytesStr, BLKSIZE, stdin);
        nBytesStr[strlen(nBytesStr) - 1] = '\0'; // kill newline
        nbytes = atoi(nBytesStr);
    }
    else if (argc < 2)
    { // they supplied a file descriptor, but no number of bytes
        fd = atoi(argv[0]);
        printf("my_read: enter number of bytes to read from file: ");
        fgets(nBytesStr, BLKSIZE, stdin);
        nBytesStr[strlen(nBytesStr) - 1] = '\0'; // kill newline
        nbytes = atoi(nBytesStr);
    }
    else
    { // they supplied everything
        fd = atoi(argv[0]);
        nbytes = atoi(argv[1]);
        //printf("fd:%d nbytes:%d", fd, nbytes);
    }

    // Verify fd is indeed open/valid and open for WR, RW, or APPEND
    int result = fd_is_valid(fd); // returns 0 if valid; -1 otherwise
    if (result)
        return -1;
    if (running->fd[fd]->mode == 1 || running->fd[fd]->mode == 3)
    {
        printf("my_read: invalid mode\n");
        return -1;
    }

    return sw_kl_read(fd, buf, nbytes);
}

/* Precondition: fd is valid*/
int sw_kl_read(int fd, char buf[], int nbytes)
{
    int logical_block, startByte, blk, avil, fileSize, offset, remain, count = 0; //avil is filesize-offset
    char ibuf[BLKSIZE] = {0}, doubleibuf[BLKSIZE] = {0}, writebuf[BLKSIZE] = {0};
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    fileSize = oftp->mptr->INODE.i_size; //file size
                                         // int count2=0;
    avil = fileSize - (oftp->offset);
    printf("sw_kl_read: available bytes to read:%d\n", avil);
    // use cq to iterate over buf
    char *cq = buf;

    if (nbytes > avil) // can't read more than the file size available
    {
        nbytes = avil;
        //printf("avil:%d\n", avil);
        // return 0;
    }

    while (nbytes > 0 && avil > 0) //read until we read the amount we were supposed to
    {

        // compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
        // this part is necessary because we might be appending...
        // 0-11 for direct, 12-(256+12) for indirect, 268-(256^2 + 12) for double indirect
        logical_block = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE; // where we start reading in the block

        /* This part of the code gets the correct block number to write to AND it allocates
		any new disc blocks if needed.
		*/
        if (logical_block < 12)
        { // direct block
            //dont need to check if the block exists for read, we assume it does
            blk = mip->INODE.i_block[logical_block]; // blk should be a disk block now [page 348]
        }
        else if (logical_block >= 12 && logical_block < 256 + 12)
        { // INDIRECT blocks

            // get i_block[12] into an int ibuf[256];
            memset(ibuf, '\0', BLKSIZE); // this is 1024 bytes, but only use first 256
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            blk = ibuf[logical_block - 12]; //actual offset
                                            //dont need to check if block exists
        }
        else
        { // double indirect blocks
            // First check if we even have the ptr array to keep track of double indirect blocks; if not, allocate block
            // get i_block[13] into an int ibuf[256];
            memset(ibuf, '\0', BLKSIZE); // this is 1024 bytes, but only use first 256
            get_block(mip->dev, mip->INODE.i_block[13], ibuf);
            // get block number. subtract num of indirect+direct blocks, THEN divide by 256 (to account for indirect block SPACE)
            blk = ibuf[(logical_block - (BLKSIZE / sizeof(int)) - 12) / (BLKSIZE / sizeof(int))];

            // so NOW we get the correct block number and allocate any more blocks if necessary
            memset(doubleibuf, '\0', BLKSIZE); // this is 1024 bytes, but only use first 256
            get_block(mip->dev, blk, doubleibuf);
            blk = ibuf[(logical_block - (BLKSIZE / sizeof(int)) - 12) % (BLKSIZE / sizeof(int))];
        }

        char rbuf[BLKSIZE];
        /* all cases come to here : read the data block */
        get_block(mip->dev, blk, rbuf); // read disk block into rbuf[ ]
        //printf("sw_kl_read rbuf: %s\n", rbuf);
        char *cp = rbuf + startByte;  // cp points at startByte in rbuf[]
        remain = BLKSIZE - startByte; // number of BYTEs remain in this block

        // Optimized read code to write full chunk w/o loop.
        //reading
        if (nbytes <= remain)
        {
            //printf("DEBUG: in if line 132\n");
            memcpy(cq, cp, nbytes); //read nbytes left
            count += nbytes;
            oftp->offset += nbytes;
            avil -= nbytes;
            remain -= nbytes;
            cq += nbytes;
            cp += nbytes;
            nbytes = 0;
        }
        else
        {
            //printf("DEBUG: in else line 144\n");
            memcpy(cq, cp, remain);
            count += remain;
            oftp->offset += remain;
            avil -= remain;
            nbytes -= remain;
            cq += remain;
            cp += remain;
            remain = 0;
        }
    }
    printf("sw_kl_read: ECHO=%s\n", buf);
    printf("my_read: read %d char from file descriptor fd=%d\n", count, fd);

    return count;
}