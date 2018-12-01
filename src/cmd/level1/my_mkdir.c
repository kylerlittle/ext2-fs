#include "../../util/util.h" //will be included in header
#include "my_mkdir.h"

//THIS ENTIRE FUNCTION IS EXPLAINED IN PAGE 332
void mk_dir(char path[MAX_FILENAME_LEN])
{
    MINODE *minodePtr;
    INODE *inodePtr;
    int ino;
    int i;
    char buf[BLKSIZE]; //BLKSIZE is just size 1024
    char t1[BLKSIZE];
    char t2[BLKSIZE];
    char parent[BLKSIZE];
    char child[1024];

    strcpy(t1, path); //holds the pathname for us
    strcpy(t2, path);

    //dirname holds pathname up to final /
    strcpy(parent, dirname(t1);
    //basename returns following last /
    strcpy(child, basename(t2));

    //need to know the parents INODE number
    ino=getino(running->cwd, parent); //returns current working directory
    printf("Inode: %d\n", ino);
    minodePtr=iget(dev,ino); //refer to page 323 in sys programming book
    inodePtr=&minodePtr->INODE

    if(!S_ISDIR(inodePtr->i_mode)) //imode field first 4 bits is file type,1000=REGfile 0100=DIR
    {
        printf("ERROR: The parent is not a directory\n");
        return;
    }

    if(getino(running->>cwd, path)!=0)
    {
        printf("ERROR: %s is already a directory\n", path);
        return;
    }

    if(!minodePtr) //checks if the parent exist
    {
        printf("ERROR: DIRECTORY DOES NOT EXIST\n");
        return;
    }

    sw_kl_mkdir(minodePtr,child); //making directory

    //so as we are making the directory, we need to keep track of the links, and make it dirty
    inodePtr->i_links_count++;
    inodePtr->i_atime=time(0L); //return type long
    minodePtr->dirty=1; //page 334 dirty flag

    iput(minodePtr); //this releases a used inode pointed to by minodePtr
    return;
}

//refer to page 334 for enter name function in system programming (4(4))

int enter_name(MINODE *minodePtr, int ino, char *name)
{
    INODE *parent_inodePtr;
    parent_inodePtr = &minodePtr->INODE;
    char buf[BLKSIZE] char *cp;
    DIR *dp;
    int bno = 0;
    int need_length = 0;
    int ideal_length = 0;
    int remain;
    //can assume only 12 direct blocks
    for (int i = 0; i < parent_inodePtr->i_size / BLKSIZE; i++)
    {
        if (parent_inodePtr->i_block[i] == 0) //page 334
        {
            break;
        }
        bno = parent_inodePtr->i_block[i];
        get_block(dev, bno, buf);
        dp = (DIR *)buf;
        cp = buf;

        //need length from page 335
        //all dir entries rec_len=ideal length except the last entry
        //the rec_length of the last entry may be longer than the ideal length

        need_length = 4 * ((8 + strlen(name) + 3) / 4); //->proves its a multiple of 4
        printf("need_length: %d\n", need_length);
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len; //rec_length is 12?? last block
            dp = (DIR *)cp;
        }
        //now we are at the last block
        printf("last_entry: %s\n", dp->name);
        cp = (char *)dp;

        //we handle this later, the last entry may not be the ideal length, we will trim it below
        ideal_length = 4 * ((8 + dp->name_len + 3) / 4);

        //now whats remaining is remain which is the last entry rec_len-ideal_length
        remain = dp->rec_length - ideal_length;
        printf("remain: %d\n", remain);
        if (remain >= need_length)
        {
            //we need to trim it to its ideal length
            dp->rec_len = ideal_length;
            cp += dp->rec_len;
            dp = (DIR *)cp;

            dp->inode = ino;
            dp->rec_len = block_size - ((u32)cp - (u32)buf);
            printf("rec_len: %d\n", dp->rec_len);
            dp->name_len = strlen(name);
            dp->file_type = EXT2_FT_DIR; //this is 2, and indicates directory file
            strcpy(dp->name, name);
            //writes
            put_block(dev, bno, buf);
            return 1;
        }
    }

    printf("Number of Data Blocks: %d\n", i);
    
    dp->inode=ino;
    dp->rec_len=BLKSIZE; //again BLKSIZE is 1024 macro defined
    dp->name_len=strlen(name);
    dp->file_type=EXT2_FT_DIR; //this is 2, and indicates directory file
    strcpy(dp->name, name);

    //now we write back, but we have updated all variables
    put_block(dev, bno, buf);
    return 1;
}
//refer to page 332 11.8.1
int sw_kl_mkdir(MINODE *minodePtr, char *child)
{
    int ino=ialloc(dev);
    int bno=balloc(dev);

    printf("dev: %d\n", dev);
    printf("ino:%d \t bno: %d\n", ino, bno);

    MINODE *minodePtr=iget(dev,ino); //load inode into minode
    INODE *inodePtr=&minodePtr->INODE;

    char *cp;
    DIR *dp;
    char buf[BLKSIZE]; //1024 size again

    //page 334 create inode reference
    inodePtr->i_mode=0x41ED; //040755: DIR TYPE PERMISSIONS
    inodePtr->i_uid=running->uid; //owner uid
    inodePtr->i_gid=running->gid; //group id
    inodePtr->i_size=BLKSIZE; //size in bytes
    ip->i_links_count=2; //links count is 2 you have . and ..
    ip->i_atime=ip->i_ctime=ip->i_mtime=time(0L);
    ip->i_blocks=2; //block count in 512 byte chunks
    ip->i_block[0]=bno; //the new directory has one data block
    for (int i=1;i<15;i++)
    {
        ip->i_block[i]=0;
    }
    mip->dirty=1;
    iput(mip);

    //we need to include . and .. page 334 (4).3
    get_block(dev, bno, buf);
    dp=(DIR*)buf;
    cp=buf;

    dp->inode=ino;
    dp->rec_len=12;
    dp->name_len=1;
    dp->file_type=(u8)EXT2_FT_DIR;
    dp->name[0]='.';
    cp+=dp->rec_len;
    dp=(DIR*)cp;

    dp->inode=minodePtr->ino;
    dp->rec_len=BLKSIZE-12;
    dp->name_len=2;
    dp->file_type=(u8)EXT2_FT_DIR;
    dp->name[0]=d->name[1]='.';

    //write to block on disk
    put_block(dev,bno,buf);
    enter_name(minodePtr, ino, child); //puts into the parents directory
    return 1;

}
