#include "my_open.h"
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

int my_open(int argc, char *argv[])
{
	// 1. ask for a pathname and mode to open:
	int mode = 0;
	char filename[MAX_FILENAME_LEN], mode_str[4];
	if (argc < 1) {  // they supplied no args
		printf("my_open: enter filename : ");
		fgets(filename, MAX_FILENAME_LEN, stdin);
  		filename[strlen(filename) - 1] = '\0';
		printf("my_open: enter mode 0|1|2|3 for R|W|RW|APPEND: ");
		fgets(mode_str, 4, stdin);
  		mode_str[strlen(mode_str) - 1] = '\0';
		mode = atoi(mode_str);
	} else if (argc < 2) {   // they supplied a file name, no mode
		printf("my_open: enter mode 0|1|2|3 for R|W|RW|APPEND: ");
		fgets(mode_str, 4, stdin);
  		mode_str[strlen(mode_str) - 1] = '\0';
		mode = atoi(mode_str);
		strcpy(filename, argv[0]);
	} else {  // they supplied everything
		strcpy(filename, argv[0]);
		mode = atoi(argv[1]);
	}
	return sw_kl_open(filename, mode);
}

int sw_kl_open(char *filename, int mode) {
	// 2. get pathname's inumber:
	int dev;
	if (filename[0]=='/') dev = root->dev;          // root INODE's dev
	else dev = running->cwd->dev;  
	int ino = getino(&dev, filename);

	if (ino == 0) {
		printf("my_open: %s doesn't exist so creating it\n", filename);
		int r = sw_kl_creat(filename);
		if (r == 0) printf("my_open: %s created successfully\n", filename);
		ino = getino(&dev, filename);
	}
	
	// 3. get its Minode pointer
	MINODE *mip = iget(dev, ino);  
	
	// 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
	if (!S_ISREG(mip->INODE.i_mode)) {
		printf("my_open: ERROR -- %s isn't a regular file\n", filename);
		return -1;
	} 
	if (!(mip->INODE.i_uid == running->uid || running->uid)) {
		printf("my_open: ERROR -- uid %d doesn't have permission\n", running->uid);
		return -1;
	}
	if (!(mip->INODE.i_gid == running->gid || running->gid)) {
		printf("my_open: ERROR -- gid %d doesn't have permission\n", running->gid);
		return -1;
	}
	// Check whether the file is ALREADY opened with INCOMPATIBLE mode:
	// If it's already opened for W, RW, APPEND : reject. (that is, only multiple R are OK)
	int i = 0, lowest_open_fd;
	for (i=0; i<NFD;++i) {
		if (running->fd[i] == NULL) {
			lowest_open_fd = i;
			break;
		}
		// Otherwise, check if already opened with incompatible mode
		if (running->fd[i]->mptr == mip) {
			if (mode > 0) {
				printf("my_open: ERROR -- %s already opened with incompatible mode\n", filename);
				return -1;
			}
		}
	}
	
	// 5. allocate a FREE OpenFileTable (OFT) and fill in values:
	OFT *oftp = (OFT *)malloc(sizeof(OFT));
	oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->mptr = mip;  // point at the file's minode[]
	
	// 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
    switch(mode){
		case 0 : 
			oftp->offset = 0;     // R: offset = 0
			break;
		case 1 : truncate(mip);        // W: truncate file to 0 size
			oftp->offset = 0;
			break;
        case 2 : 
			oftp->offset = 0;     // RW: do NOT truncate file
			break;
		case 3 :
			oftp->offset =  mip->INODE.i_size;  // APPEND mode
            break;
        default: printf("invalid mode\n");
			return -1;
      }
	  
	// 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
	// Let running->fd[i] point at the OFT entry
	running->fd[lowest_open_fd] = oftp;
	
	// 8. update INODE's time field
    //      for R: touch atime. 
    //      for W|RW|APPEND mode : touch atime and mtime
    //    mark Minode[ ] dirty
	if (mode > 0) {
		mip->INODE.i_mtime = time(NULL); //updates the file modification time
	}
	mip->INODE.i_atime = time(NULL); //updates the file access time
	mip->dirty = 1;
	iput(mip);

   	// 9. return i as the file descriptor
	printf("my_open: %s opened successfully; fd number is %d\n", filename, lowest_open_fd);
	return lowest_open_fd;
}