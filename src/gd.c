/********* gd.c code ***************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

/******************* in <ext2fs/ext2_fs.h>*******************************
struct ext2_group_desc
{
	u32	bg_block_bitmap;	// Blocks bitmap block
	u32	bg_inode_bitmap;	// Inodes bitmap block
	u32	bg_inode_table;		// Inodes table block
	u16	bg_free_blocks_count;	// Free blocks count
	u16	bg_free_inodes_count;	// Free inodes count
	u16	bg_used_dirs_count;	    // Directories count
	u16	bg_flags;
	u32	bg_exclude_bitmap_lo;	// Exclude bitmap for snapshots
	u16	bg_block_bitmap_csum_lo;// crc32c(s_uuid+grp_num+bitmap) LSB
	u16	bg_inode_bitmap_csum_lo;// crc32c(s_uuid+grp_num+bitmap) LSB
	u16	bg_itable_unused;	// Unused inodes count
	u16	bg_checksum;		// crc16(s_uuid+group_num+group_desc)
};
**********************************************************************/

char buf[BLKSIZE];
int fd;

int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd, (long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int gd()
{
    // read GROUP DESCRIPTOR block
    get_block(fd, 2, buf);  
    gp = (GD *)buf;

    if (!gp) {
        printf("No GROUP DESCRIPTOR block!\n");
        exit(1);
    }
    printf("EXT2 FS OK\n");

    printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);
    printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);

    printf("bg_inode_table = %d\n", gp->bg_inode_table);

    printf("bg_free_inodes_count = %d\n", gp->bg_free_inodes_count);
    printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count);
    printf("bg_used_dirs_count = %d\n", gp->bg_used_dirs_count);

    printf("bg_flags = %d\n", gp->bg_flags);
    printf("bg_exclude_bitmap_lo = %d\n", gp->bg_exclude_bitmap_lo);
    printf("bg_inode_bitmap_csum_lo = %d\n", gp->bg_inode_bitmap_csum_lo);
    printf("bg_block_bitmap_csum_lo = %d\n", gp->bg_block_bitmap_csum_lo);

    printf("bg_itable_unused = %d\n", gp->bg_itable_unused);
    printf("bg_checksum = %d\n", gp->bg_checksum);
}

char *disk = "../mydisk";

int main(int argc, char *argv[ ])
{ 
    if (argc > 1)
    disk = argv[1];
    fd = open(disk, O_RDONLY);
    if (fd < 0){
        printf("open failed\n");
        exit(1);
    }
    gd();
}