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
    printf("fd\t mode\t offset\t device\t inode\n");
    printf("--\t ----\t ------\t ------\t -----\n");

    for(int i=0;i<NFD;i++) //NOFT is defined as 40
    {
        if (running->fd[i] == NULL) break;
        openFilePointer=running->fd[i];
        if(openFilePointer->refCount==0)
        {
            return;
        }
        printf("%2d\t ",i);

        if(openFilePointer->mode==0)
        {
            printf("READ");
        }
        else if(openFilePointer->mode==1)
        {
            printf("WRITE");
        }
        else if(openFilePointer->mode==2)
        {
            printf("READ/WRITE");
        }
        else if(openFilePointer->mode==3)
        {
            printf("APPEND");
        }
        else
        {
            printf("-----");
        }
        printf("\t %6d\t %6d\t %5d\n", openFilePointer->offset, openFilePointer->mptr->dev, openFilePointer->mptr->ino);
    }
}