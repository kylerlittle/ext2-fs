#include "my_cp.h"
#include "my_open.h"
#include "my_close.h"
#include "my_read.h"
#include "my_write.h"
#include "../level1/my_creat.h"

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

int my_cp(int argc, char *argv[])
{
	// 1. argv[0] is src; argv[1] is dst (where we copying to!)
	if (argc < 2) {
		printf("my_cp: ERROR -- need two files to proceed\n");
		return -1;
	}
	char src[MAX_FILENAME_LEN], dst[MAX_FILENAME_LEN];
	strcpy(src, argv[0]); strcpy(dst, argv[1]);
	return sw_kl_cp(src, dst);
}

int sw_kl_cp(char *src, char *dst) {
	int src_fd, dst_fd;
	src_fd = sw_kl_open(src, 0);  // mode 0 is READ;
	dst_fd = sw_kl_open(dst, 2);  // mode 2 is READ/WRITE

	if (src_fd == -1 || dst_fd == -1) {
		if (src_fd == -1) sw_kl_close(src_fd);
		if (dst_fd == -1) sw_kl_close(dst_fd);
		printf("my_cp: open %s or open %s failed\n", src, dst);
		return -1;
	}

	// 2. Read/write!
	int n=0;
	char buf[BLKSIZE];
	memset(buf, '\0', BLKSIZE);
	while ((n=sw_kl_read(src_fd, buf, BLKSIZE)) != 0){
       sw_kl_write(dst_fd, buf, n);  // notice the n in write()
	   memset(buf, '\0', BLKSIZE);
   	}

	// 3. Close file descriptors
	sw_kl_close(src_fd);
	sw_kl_close(dst_fd);
	return 0;
}