#include "my_creat.h"

int my_creat(int argc, char *argv[])
{
	int i;
	for (i=0;i<argc;i++) {
		if (sw_kl_creat(argv[i])) {
			printf("my_creat: ERROR -- stopped after %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}

int creat_child_under_pmip(MINODE *parent_minodePtr, char *child_name)
{
	int i;
	//allocate inode for new file
	int ino = ialloc(dev);

	//create it in memory
	MINODE *minodePtr = iget(dev, ino);
	INODE *inodePtr = &minodePtr->INODE;

	//set the variables and do NOT incrememnt parents link count
	inodePtr->i_mode = 0x81A4; //file type or 0100644 in mkdir_creat notes
	inodePtr->i_uid  = running->uid; // Owner uid
    inodePtr->i_gid  = running->gid; // Group Id
    inodePtr->i_size = 0;	// Size in bytes-->no data block
    inodePtr->i_links_count = 1; // # links to parent directory
    inodePtr->i_atime = inodePtr->i_ctime=inodePtr->i_mtime = time(0L); // Set to current time
	
	

	inodePtr->i_blocks = 0;

	//set data blocks to 0
	for(i = 0; i < 15; i++)
	{
		inodePtr->i_block[i] = 0;
	}

	//mark dirty and write back
	minodePtr->dirty = 1;
	iput(minodePtr);

	//enters new file's name in parent directory
	enter_name(parent_minodePtr, ino, child_name);//enter name defined in mkdir.c

	//return the ino
	return ino;
}

int sw_kl_creat(char *filename) {
	int i, ino;
	MINODE *parent_minodePtr;
	INODE *parent_inodePtr;

	char buf[BLKSIZE];
	char t1[BLKSIZE], t2[BLKSIZE];
	char parent_name[BLKSIZE], child_name[BLKSIZE];

	strcpy(t1, filename);
	strcpy(t2, filename);

	//get basename and dirname of argv[0]
	strcpy(child_name, basename(t2));
	strcpy(parent_name, dirname(t1));
	

	//get ino of parent
	ino = getino(&running->cwd->dev, parent_name);
	//printf("%d\n", ino);
	parent_minodePtr = iget(dev, ino);
	parent_inodePtr = &parent_minodePtr->INODE;

	//check if the parent exists
	if(!parent_minodePtr)
	{
		printf("ERROR: Parent does not exist\n");
		return -1;
	}

	//check if directory
	if(!S_ISDIR(parent_inodePtr->i_mode))
	{
		printf("ERROR: Parent is not a directory\n");
		return -1;
	}

	//check if dir exists
	if(getino(&running->cwd->dev, filename) != 0)
	{
		printf("ERROR: %s already exists\n", filename);
		return -1;
	}
	
	creat_child_under_pmip(parent_minodePtr, child_name);

	//size = 0, linkcount = 1, don't increment parent's links count
	parent_inodePtr->i_atime = time(0L);

	parent_minodePtr->dirty = 1;

	iput(parent_minodePtr);

	return 0;
}
