#ifndef CONSTANTS
#define CONSTANTS

/* Debugging Mode */
#define DEBUG_MODE 0

/* IO */
#define BLKSIZE 1024

/* Define shorter TYPES, save typing efforts */
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

/* inode Constants */
#define NUM_DIRECT_BLKS 12

/* Macro -- String Lengths */
#define MAX_FILENAME_LEN 255

#endif