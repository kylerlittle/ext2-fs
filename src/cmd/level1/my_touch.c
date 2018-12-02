#include "my_touch.h"


int my_touch(int argc, char *argv[])
{
    sw_kl_touch(argv[0]); 
}

void sw_kl_touch(char *pathname)
{
        printf("Touching file %s\n", pathname);
        int ino;
		MINODE *minodePtr;
		ino = getino(&fd, pathname);
		minodePtr = iget(fd, ino);
		minodePtr->INODE.i_mtime = time(NULL); //updates the file modification time
		minodePtr->dirty = 1;
		iput(minodePtr);
}