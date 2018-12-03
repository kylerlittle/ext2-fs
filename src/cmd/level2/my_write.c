#include "my_write.h"

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

/* Remove surrounding quotation marks of src (if any) and copy to src. */
int strip_quotes(char *dst, char *src) {
	int last = strlen(src) - 1;
	// Remove last quote if it exists
	if (src[last] == '"') src[last] = 0;
	// Now remove first quote if it exists and copy to dst
	if (src[0] == '"') {
		strcpy(dst, &src[1]);
	} else {
		strcpy(dst, src);
	}
	printf("strip_quotes: ECHO=%s\n", dst);
	return 0;
}

// Text is in argv[0] to argv[argc-1]
int get_text(char *buf, int argc, char *argv[]) {
	char temp[BLKSIZE];
	strcpy(temp, argv[0]);
	strcat(temp, " ");
	int i;
	for (i=1;i<argc;i++) {
		strcat(temp, argv[i]);
		if (i+1!=argc) strcat(temp, " ");  // add space if not last char
	}
	strip_quotes(buf, temp);
	return 0;
}

int my_write(int argc, char *argv[])
{
	// 1. ask for a fd and text to write in file
	int fd = 0;
	char fd_str[MAX_FILENAME_LEN], text[BLKSIZE], buf[BLKSIZE];
	if (argc < 1) {  // they supplied no args
		printf("my_write: enter opened file descriptor to write to : ");
		fgets(fd_str, MAX_FILENAME_LEN, stdin);
  		fd_str[strlen(fd_str) - 1] = '\0';
		fd = atoi(fd_str);
		printf("my_write: enter text to write to file: ");
		fgets(text, BLKSIZE, stdin);
  		text[strlen(text) - 1] = '\0';  // kill newline
		strip_quotes(buf, text);
	} else if (argc < 2) {   // they supplied a file descriptor, but no text
		fd = atoi(argv[0]);
		printf("my_write: enter text to write to file: ");
		fgets(text, BLKSIZE, stdin);
  		text[strlen(text) - 1] = '\0';  // kill newline
		strip_quotes(buf, text);
	} else {  // they supplied everything
		fd = atoi(argv[0]);
		get_text(buf, argc-1, &argv[1]);
	}

	// Verify fd is indeed open/valid and open for WR, RW, or APPEND
	int result = fd_is_valid(fd); // returns 0 if valid; -1 otherwise
	if (result) return -1;
	if (running->fd[fd]->mode == 0) {
		printf("my_write: invalid mode\n");
		return -1;
	}

	return sw_kl_write(fd, buf, strlen(buf));
}

/* Precondition: fd is valid and opened for write */
int sw_kl_write(int fd, char buf[], int nbytes) {
	printf("sw_kl_write: ECHO=%s\n", buf);
	int logical_block, startByte, blk, remain;
	char ibuf[BLKSIZE], doubleibuf[BLKSIZE], writebuf[BLKSIZE];
	OFT *oftp = running->fd[fd];
	MINODE *mip = oftp->mptr;

	// use cq to iterate over buf
	char *cq = buf;

	while (nbytes > 0 ) {
		// compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
		// this part is necessary because we might be appending...
		// 0-11 for direct, 12-(256+12) for indirect, 268-(256^2 + 12) for double indirect
		logical_block = oftp->offset / BLKSIZE;  
		startByte = oftp->offset % BLKSIZE;  // where we start writing in the block
		
		/* This part of the code gets the correct block number to write to AND it allocates
		any new disc blocks if needed.
		*/
		if (logical_block < 12){                         // direct block
			if (mip->INODE.i_block[logical_block] == 0) {   // if no data block yet
				mip->INODE.i_block[logical_block] = balloc(mip->dev);// MUST ALLOCATE a block
			}
			blk = mip->INODE.i_block[logical_block];      // blk should be a disk block now
		}
		else if (logical_block >= 12 && logical_block < 256 + 12){ // INDIRECT blocks 
			// First check if we even have the ptr array to keep track of indirect blocks; if not, allocate block
			if (mip->INODE.i_block[12] == 0) {  // ONLY IF FIRST INDIRECT BLOCK ALLOCATION
				// allocate a block for it;
				mip->INODE.i_block[12] = balloc(mip->dev);
				// zero out the block on disk !!!!
				get_block(mip->dev, mip->INODE.i_block[12], ibuf);
				int *ip = (int*)ibuf, p=0;  // step thru block in chunks of sizeof(int), set each ptr to 0
				for (p=0; p<BLKSIZE/sizeof(int);p++) ip[p] = 0;
				put_block(mip->dev, mip->INODE.i_block[12], ibuf); // write back to disc
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// get i_block[12] into an int ibuf[256];
			memset(ibuf, '\0', BLKSIZE);  // this is 1024 bytes, but only use first 256
			get_block(mip->dev, mip->INODE.i_block[12], ibuf);
			blk = ibuf[logical_block - 12];
			if (blk==0){
				// allocate a disk block;
				blk = ibuf[blk] = balloc(mip->dev);
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// record it in i_block[12];
			put_block(mip->dev, mip->INODE.i_block[12], ibuf); // write back to disc
		}
		else { // double indirect blocks
			// First check if we even have the ptr array to keep track of double indirect blocks; if not, allocate block
			if (mip->INODE.i_block[13] == 0) {  // ONLY IF FIRST DOUBLE INDIRECT BLOCK ALLOCATION
				// allocate a block for it;
				mip->INODE.i_block[13] = balloc(mip->dev);
				// zero out the block on disk !!!!
				get_block(mip->dev, mip->INODE.i_block[13], ibuf);
				int *ip = (int*)ibuf, p=0;  // step thru block in chunks of sizeof(int), set each ptr to 0
				for (p=0; p<BLKSIZE/sizeof(int);p++) ip[p] = 0;
				put_block(mip->dev, mip->INODE.i_block[13], ibuf); // write back to disc
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// get i_block[13] into an int ibuf[256];
			memset(ibuf, '\0', BLKSIZE);  // this is 1024 bytes, but only use first 256
			get_block(mip->dev, mip->INODE.i_block[13], ibuf);
			// get block number. subtract num of indirect+direct blocks, THEN divide by 256 (to account for indirect block SPACE)
			blk = ibuf[(logical_block - (BLKSIZE/sizeof(int)) - 12)/(BLKSIZE/sizeof(int))];
			if (blk==0){
				// allocate a disk block;
				blk = ibuf[blk] = balloc(mip->dev);
				// zero out the block on disk !!!!
				get_block(mip->dev, blk, doubleibuf);
				int *ip = (int*)doubleibuf, p=0;  // step thru block in chunks of sizeof(int), set each ptr to 0
				for (p=0; p<BLKSIZE/sizeof(int);p++) ip[p] = 0;
				put_block(mip->dev, blk, doubleibuf); // write back to disc
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// record it in i_block[13];
			put_block(mip->dev, mip->INODE.i_block[13], ibuf); // write back to disc
			// at i_block[13], we have array of pointer. each pointer points to another table. blk is num of first nonfull table
			// so NOW we get the correct block number and allocate any more blocks if necessary
			memset(doubleibuf, '\0', BLKSIZE);  // this is 1024 bytes, but only use first 256
			get_block(mip->dev, blk, doubleibuf);
			blk = ibuf[(logical_block - (BLKSIZE/sizeof(int)) - 12)%(BLKSIZE/sizeof(int))];
			if (blk==0){
				// allocate a disk block;
				blk = doubleibuf[blk] = balloc(mip->dev);
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// record it
			put_block(mip->dev, blk, doubleibuf); // write back to disc
		}

		char wbuf[BLKSIZE];
		/* all cases come to here : write to the data block */
		get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
		char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
		remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

		// Optimized write code to write full chunk w/o loop.
		// write 'remain' bytes to block if remain <= nbytes; of course, if nbytes < remain, simply write nbytes
		int amount_to_write = (remain <= nbytes) ? remain : nbytes;
		// write!!! recall cp points out where we start write (wbuf+startByte); cq points at buf
		memcpy(cp, cq, amount_to_write);
		// Update control vars
		cp = cq = cp+amount_to_write;
		oftp->offset += amount_to_write;
		if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
			mip->INODE.i_size = oftp->offset;    // update file size to offset since offset points to after what was just written
		nbytes -= amount_to_write;
		put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
		
		// loop back to outer while to write more .... until nbytes are written
	}

	mip->dirty = 1;       // mark mip dirty for iput() 
	printf("my_write: wrote %d char into file descriptor fd=%d\n", strlen(buf), fd);           
	return nbytes;
}