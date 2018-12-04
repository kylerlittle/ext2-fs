#include "my_close.h"

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

int my_close(int argc, char *argv[])
{
	// argv[0] is fd user wishes to close

	// 1. Check whether fd is valid to close.
	if(argc<1)
	{
		printf("ERROR: you must provide a file descriptor\n");
		return -1;
	}
	int fd_to_close = atoi(argv[0]);
	
	int result = fd_is_valid(fd_to_close); // returns 0 if valid; -1 otherwise
	if (result) return -1;
	return sw_kl_close(fd_to_close);
}

int sw_kl_close(int fd) {
	// 2. fd is valid, so set ptr to NULL in table, decrement refCount
	OFT *oftp = running->fd[fd];
    running->fd[fd] = NULL;
    oftp->refCount--;
    if (oftp->refCount > 0) return 0;

    // 3. If last user of this OFT entry ==> dispose of the minode & free malloc'd memory
    MINODE *mip = oftp->mptr;
    iput(mip);

	free(oftp);
	printf("my_close: last user of fd, so free'd malloc'd mem\n");

    return 0;
}