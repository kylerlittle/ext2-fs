#include "my_rmdir.h"

//11.8.4 page 338

int isEmpty(MINODE *minodePtr) //from page 338 in systems programming
{
    // //is empty will return 1 if not empty, 0 if empty
    // char buf[BLKSIZE];
    // INODE *inodePtr=&minodePtr->INODE;
    // char *cp;
    // char name[64];
    // DIR *dp;

    // if(ip->i_links_count>2)
    // {
    //     return 1;
    // }
    // //every DIRs link count starts with 2 for . and ..
    // //each subdir increases link count by 1
    // //reg files do not incremenet
    // //if the link count is 2, the DIR may have reg files
    // //traverse DIRs data blocks to count dir entries
    // //must be greater than 2
    // else if(ip->i_links_count==2) 
    // {
    //     if (ip->i_block[1])
    //     {
    //         get_block(dev, ip->i_block[1], buf);
    //         cp=buf;
    //         dp=(DIR*)buf;
    //         while(cp<buf+BLKSIZE)
    //         {
    //             strncpy(name, dp->name, dp->name_len); //for n in strncpy
    //             name[dp->name_len]=0;

    //             if(strcmp(name,".")!=0 && strcmp(name, "..")!=0)
    //             {
    //                 //if the name is not . or .., then there is a subdir, need to check
    //                 return 1;
    //             }
    //         }

    //     }
    if (1){
    }
    else
    {
        return 0; //-->this means empty
    }
}
//page 338
void rm_child(MINODE *parent_minodePtr, char *name)
{
    // int i;
    // char buf[BLKSIZE];
    // char *cp;
    // char *final_cp;
    // int first;
    // int last;
    // char temp[64];
    // INODE *parent_inodePtr=&parent->INODE;
    // DIR *dp;
    // DIR *predecessor_dp;
    // DIR *last_entry_dp;
    
    // printf("Attempting to remove %s\n", name);

    // for(i=0;i<12;i++)
    // {
    //     if(parent_inodePtr->i_block[i]==0)
    //     {
    //         return 0;
    //     }
    //     get_block(dev, parent_inodePtr->i_block[i], buf);
        
    // }

}

int my_rmdir(int argc, char *argv[]) {

}