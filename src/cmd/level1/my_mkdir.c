#include "my_mkdir.h"

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

/* Mimic linux mkdir; if passed multiple args, remove all. */
int my_mkdir(int argc, char*argv[])
{
    int i;
    for (i=0; i < argc; i++) {
        mk_dir(argv[i]);
    }
}

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
    char child[BLKSIZE];

    strcpy(t1, path); //holds the pathname for us
    strcpy(t2, path);

    //dirname holds pathname up to final /
    strcpy(parent, dirname(t1));
    //basename returns following last /
    strcpy(child, basename(t2));

    //need to know the parents INODE number
    ino=getino(running->cwd, parent); //returns current working directory
    printf("Inode: %d\n", ino);
    minodePtr=iget(dev,ino); //refer to page 323 in sys programming book
    inodePtr=&minodePtr->INODE;

    if(!S_ISDIR(inodePtr->i_mode)) //imode field first 4 bits is file type,1000=REGfile 0100=DIR
    {
        printf("ERROR: The parent is not a directory\n");
        return;
    }

    if(getino(running->cwd, path)!=0)
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

//refer to page 332 11.8.1
int sw_kl_mkdir(MINODE *parent_minodePtr, char *child)
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
    inodePtr->i_links_count=2; //links count is 2 you have . and ..
    inodePtr->i_atime=inodePtr->i_ctime=inodePtr->i_mtime=time(0L);
    inodePtr->i_blocks=2; //block count in 512 byte chunks
    inodePtr->i_block[0]=bno; //the new directory has one data block
    for (int i=1;i<15;i++)
    {
        inodePtr->i_block[i]=0;
    }
    minodePtr->dirty=1;
    iput(minodePtr);

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

    dp->inode=parent_minodePtr->ino;
    dp->rec_len=BLKSIZE-12;
    dp->name_len=2;
    dp->file_type=(u8)EXT2_FT_DIR;
    dp->name[0]=dp->name[1]='.';

    //write to block on disk
    put_block(dev,bno,buf);
    enter_name(parent_minodePtr, ino, child); //puts into the parents directory
    return 1;

}
