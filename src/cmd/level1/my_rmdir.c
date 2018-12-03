#include "my_rmdir.h"

//11.8.4 page 338

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

int my_rmdir(int argc, char*argv[])
{
    int i, result;
    for (i=0; i<argc; i++) {
        result = sw_kl_rmdir(argv[i]);
        if (result) break;
    }
    
}
int isEmpty(MINODE *minodePtr) //from page 338 in systems programming
{
    //is empty will return 1 if not empty, 0 if empty
    char buf[BLKSIZE];
    INODE *inodePtr = &minodePtr->INODE;
    char *cp;
    char name[64];
    DIR *dp;

    if (inodePtr->i_links_count > 2)
    {
        return -1;
    }
    //every DIRs link count starts with 2 for . and ..
    //each subdir increases link count by 1
    //reg files do not incremenet
    //if the link count is 2, the DIR may have reg files
    //traverse DIRs data blocks to count dir entries
    //must be greater than 2
    else if (inodePtr->i_links_count == 2)
    { 
        // need to know if the number of dir entries is > 2
        /* Explanation for only checking i_block[0]: if there are in fact more files/dirs under
           the dir we wanna remove, they would be immediately after '.' and '..' in first data block.*/
        if (inodePtr->i_block[0])
        {
            //printf("dsfksjadfldsj inside\n");
            get_block(dev, inodePtr->i_block[0], buf);
            cp = buf;
            dp = (DIR *)buf;
            while (cp < buf + BLKSIZE)
            {
                strncpy(name, dp->name, dp->name_len); //for n in strncpy
                name[dp->name_len] = 0;

                if (name[0]==0) break;
                
                // name can be ".." or name can be "."; if not return -1
                if (!(strcmp(name, ".") == 0 || strcmp(name, "..") == 0))
                {
                    printf("isEmpty: %s is not . or ..\n", name);
                    //if the name is not . or .., then there is a sub file, need to check
                    return -1;
                }
                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
            return 0; // if we survive the loop, return 0
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0; //-->this means empty
    }
}

int sw_kl_rmdir(char *path)
{
    int i=0, ino, parent_ino;
    char temp[64], child[64], parentname[MAX_FILENAME_LEN];
    MINODE *minodePtr, *parent_minodePtr;
    INODE *inodePtr, *parent_inodePtr;
    
    if(!path) //must provide a path
    {
        printf("my_rmdir: ERROR -- no path\n");
        return -1;
    }
    strcpy(temp, path);
    strcpy(child,basename(temp)); //gets the child
    strcpy(parentname, dirname(temp));
    if (parentname[0] == 0) strcpy(parentname, "/");

    ino=getino(&running->cwd->dev, path); //gets the minode
    printf("[path, ino]: [%s, %d]\n", path, ino);
    minodePtr=iget(dev, ino);

    if(!minodePtr) //the minodePtr isn't pointing anywhere
    {
        printf("my_rmdir: ERROR -- The child does not exist\n");
        return -1;
    }

    if(!S_ISDIR(minodePtr->INODE.i_mode))
    {
        printf("my_rmdir: ERROR -- %s is not a directory\n", path);
        return -1;
    }

    if(isEmpty(minodePtr)) //should work, checking if not empty
    {
        printf("my_rmdir: ERROR -- %s directory is not empty\n", path);
        return -1;
    }

    //going to begin to remove
    inodePtr=&minodePtr->INODE;
    
    findino(minodePtr,&ino);
    printf("ino: %d\n", ino);
    parent_ino = getino(&dev, parentname);
    parent_minodePtr=iget(dev, parent_ino);
    parent_inodePtr=&parent_minodePtr->INODE;
    //deallocate blocks
    for(i=0;i<15&&inodePtr->i_block[i]!=0;i++)
    {
        bdealloc(dev, inodePtr->i_block[i]);
    }
    //now we need to deallocate inode
    /*when deallocating the inode, it clears the ino's
    bit in the devices inode bitmap to 0, incrmemebts the free inodes count in 
    both the superblock and group descriptor by 1*/

    idealloc(dev, ino);
    
    rm_child(parent_minodePtr, child);
    

    //now we need to update the parent
    parent_inodePtr->i_links_count--;
    parent_inodePtr->i_atime=parent_inodePtr->i_mtime=time(0L);
    parent_minodePtr->dirty=1;

    //write to disk
    iput(parent_minodePtr);

    //update deleted directory in disk
    minodePtr->dirty=1;
    iput(minodePtr);

    return 0;
}