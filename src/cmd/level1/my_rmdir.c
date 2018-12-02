#include "my_rmdir.h"

//11.8.4 page 338

int my_rmdir(int argc, char*argv[])
{
    //argv[0]
    sw_kl_rmdir(argv[0]);
    

}
int isEmpty(MINODE *minodePtr) //from page 338 in systems programming
{
    //is empty will return 1 if not empty, 0 if empty
    char buf[BLKSIZE];
    INODE *inodePtr = &minodePtr->INODE;
    char *cp;
    char name[64];
    DIR *dp;

    if (ip->i_links_count > 2)
    {
        return 1;
    }
    //every DIRs link count starts with 2 for . and ..
    //each subdir increases link count by 1
    //reg files do not incremenet
    //if the link count is 2, the DIR may have reg files
    //traverse DIRs data blocks to count dir entries
    //must be greater than 2
    else if (ip->i_links_count == 2)
    {
        if (ip->i_block[1])
        {
            get_block(dev, ip->i_block[1], buf);
            cp = buf;
            dp = (DIR *)buf;
            while (cp < buf + BLKSIZE)
            {
                strncpy(name, dp->name, dp->name_len); //for n in strncpy
                name[dp->name_len] = 0;

                if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
                {
                    //if the name is not . or .., then there is a subdir, need to check
                    return 1;
                }
            }
        }
    }
    else
    {
        return 0; //-->this means empty
    }
}

void sw_kl_rmdir(char *path)
{
    int i=0;
    int ino;
    int parent_ino;
    char temp[64];
    char child[64];
    MINODE *minodePtr;
    MINODE *parent_minodePtr;
    INODE *inodePtr;
    INODE *parent_inodePtr;
    
    if(!path) //must provide a path
    {
        printf("ERROR: No path\n");
        return;
    }
    strcpy(temp, path);
    strcpy(child,basename(temp)); //gets the child

    ino=getino(running->cwd, path); //gets the minode
    printf("[path, ino]: [%s, %d]\n", path, ino);
    minodePtr=iget(dev, ino);

    if(!minodePtr) //the minodePtr isn't pointing anywhere
    {
        printf("ERROR: The child does not exist\n");
        return;
    }

    if(isEmpty(minodePtr)) //should work, checking if not empty
    {
        printf("ERROR: Directory is not empty\n");
        return;
    }

    if(!S_ISDIR(minodePtr->INODE.i_mode))
    {
        printf("ERROR: %s is not a directory\n", path);
    }

    //going to begin to remove
    inodePtr=&minodePtr->INODE;
    
    findino(minodePtr,&ino);
    printf("ino: %d\n", ino);
    parent_minodePtr=iget(dev, parent_ino);
    parent_inodePtr=&parent_minodePtr->INODE;
    //deallocate blocks
    for(;i<15&&inodePtr->i_block[i]!=0;i++)
    {
        bdealloc(dev, ip->i_block[i]);
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

    return;
}