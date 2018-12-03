/*************** type.h file *********************************/
#ifndef __TYPE_H__
#define __TYPE_H__

/**** Included headers/libraries ****/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

/* Define shorter TYPES, save typing efforts */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

/* Global declarations (can only be declared once). */
SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;

/* More readable constants for FREE blocks/inodes/fd, etc. Additionally, use this
for "status" attribute of processes. */
#define FREE 0
#define READY 1

/* File System Constants */
#define BLKSIZE  1024
#define NMINODE    64
#define NOFT       64
#define NFD        10
#define NMOUNT      4
#define NPROC       2

/* Debugging Mode */
#define DEBUG_MODE 0

/* inode Constants */
#define NUM_DIRECT_BLKS 12

/* Macro -- String Lengths */
#define MAX_FILENAME_LEN    255
#define MAX_COMPONENTS       64
#define MAX_INPUT_LEN       512
#define MAX_CMDS             64

/* File System Data Structures */
typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntable *mptr;
}MINODE;

typedef struct oft{ //page 321
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;


typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid, gid;
  int status;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

#endif
/*********************************************************/
