#include "my_lseek.h"
#include "my_pfd.h"

//11.9.2 page 346




int sw_kl_lseek(int fd, int position)
{
    int originalPosition;
    OFT *openFileTablePtr=running->fd[fd];
    if (running->fd[fd]==NULL) //error checking, prevents us from using an fd not in pfd
    {
        fprintf(stderr, "sw_kl_lseek: fd is null\n");
        printf("Check the file descriptors:\n");
        return -1;
    }
    if(position<=openFileTablePtr->mptr->INODE.i_size)
    {
        originalPosition=openFileTablePtr->offset;
        openFileTablePtr->offset=position;
        return originalPosition;
    }
    else
    {
        fprintf(stderr, "sw_kl_lseek: offset is set past the file size\n");
        return -1;
    }
}

int my_lseek(int argc, char *argv[])
{
    
    if(argc!=2)
    {
        fprintf(stderr, "Missing operand\n");
        //shows the file descriptors that are open
        my_pfd(0, NULL);
        return -1;
    }

    char* fdStr = argv[0]; 
    int fd = atoi(fdStr); 
    printf("fd: %d\n",fd);

    char* posStr = argv[1];
    int position = atoi(posStr); 
    printf("pos: %d\n", position);
    position = sw_kl_lseek(fd, position); //update

    my_pfd(0, NULL); //follow print for file descriptors

    return position;
}