#include "my_cp.h"
#include "my_open.h"
#include "my_close.h"
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

	int src_fd, dst_fd;
	char *src_argv[2] = {"0", src}, *dst_argv[2] = {"2", dst};  // mode 0 is READ; mode 2 is READ/WRITE
	src_fd = my_open(2, src_argv);
	dst_fd = my_open(2, dst_argv);
	if (dst_fd == -1) {
		printf("my_cp: creating %s since didn't exist\n", dst);
		my_creat(1, &dst_argv[1]);
		dst_fd = my_open(2, dst_argv); // don't forget to reopen!!
	}
	char src_fd_str[32], dst_fd_str[32];
	sprintf(src_fd_str, "%d", src_fd);  // handy conversion back to string
	sprintf(dst_fd_str, "%d", dst_fd);  // handy conversion back to string
	char *src_fd_str_argv[1] = {src_fd_str}, *dst_fd_str_argv[1] = {dst_fd_str};
	if (src_fd == -1 || dst_fd == -1) {
		if (src_fd != -1) my_close(1, src_fd_str_argv);
		if (dst_fd != -1) my_close(1, dst_fd_str_argv);
		printf("my_cp: open %s or open %s failed\n", src, dst);
		return -1;
	}

	// 2. Read/write!

	// 3. Close file descriptors
	my_close(1, src_fd_str_argv);
	my_close(1, dst_fd_str_argv);
}