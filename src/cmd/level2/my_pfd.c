#include "my_pfd.h"
//refer to page 320 in systems programming for OFT struct

//OFT is open file table-->page 345 for OFT description


int my_pfd(int argc, char *argv[])
{
    sw_kl_pfd(argv[0]);
}

void sw_kl_pfd(char *filename)
{
    //int i=0;
    OFT *openFilePointer;

    printf("=====================PFD=====================\n");
    printf("fd\t mode\t offset\t device\t inode\n");
    printf("--\t ----\t ------\t ------\t -----\n");

    for(int i=0;i<NOFT;i++) //NOFT is defined as 40
    {
        openFilePointer=&oft[i];
        if(openFilePointer->refCount==0)
        {
            return;
        }
        printf("%02d",i);
    }

    if(openFilePointer->mode==0)
    {
        printf("RD");
    }
    else if(openFilePointer->mode==1)
    {
        printf("WR");
    }
    else if(openFilePointer->mode==2)
    {
        printf("RW");
    }
    else if(openFilePointer->mode==3)
    {
        printf("AP");
    }
    else
    {
        printf("--");
    }

    printf("\t %6d\t %2d\t %5d\n", openFilePointer->offset, openFilePointer->mptr->dev, openFilePointer->mptr);
}