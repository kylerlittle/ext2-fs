#include "my_lseek.h"

//11.9.2 page 346

int my_lseek(int argc, char *argv[])
{
    sw_kl_lseek(argv[0]);
}

int sw_kl_lseek(char *pathname[])
{
    int fd;
    int position;
    int originalPosition;
    OFT *openFileTablePtr;
   


    printf("in sw\n");
    fd=(pathname[0]-'0');
    position=(pathname[1]-'0');

    if(running->fd[fd]==NULL)
    {
        printf("ERROR: fd is null\n");
        return -1;
    }
    openFileTablePtr=running->fd[fd];
    if(position>openFileTablePtr->mptr->INODE.i_size)
    {
        //changing the OFT entry offset, ensuring it doesnt run over either end of the file
        printf("ERROR: out of bounds\n");
        return -1;
    }

    originalPosition=openFileTablePtr->offset;
    openFileTablePtr->offset=position;//updating offset
    return originalPosition;

}