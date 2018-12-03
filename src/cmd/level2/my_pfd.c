#include "my_pfd.h"

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

//refer to page 320 in systems programming for OFT struct
//OFT is open file table-->page 345 for OFT description

int my_pfd(int argc, char *argv[])
{
    sw_kl_pfd();
}

void sw_kl_pfd(void)
{
    //int i=0;
    OFT *openFilePointer;

    printf("*********************** PFD ************************\n");
    printf("fd\t\t mode\t offset\t device\t inode\n");
    printf("--\t\t ----\t ------\t ------\t -----\n");

    for(int i=0;i<NFD;i++) //NOFT is defined as 40
    {
        if (running->fd[i] == NULL) continue;
        openFilePointer=running->fd[i];
        if(openFilePointer->refCount==0)
        {
            return;
        }
        printf("%2d\t ",i);

        if(openFilePointer->mode==0)
        {
            printf("%12s", "READ");
        }
        else if(openFilePointer->mode==1)
        {
            printf("%12s", "WRITE");
        }
        else if(openFilePointer->mode==2)
        {
            printf("%12s", "READ/WRITE");
        }
        else if(openFilePointer->mode==3)
        {
            printf("%12s", "APPEND");
        }
        else
        {
            printf("-----");
        }
        printf("\t %6d\t %6d\t %5d\n", openFilePointer->offset, openFilePointer->mptr->dev, openFilePointer->mptr->ino);
    }
}