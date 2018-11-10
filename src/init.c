#include "init.h"

MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk, inode_start;
char line[MAX_INPUT_LEN], cmd[32], pathname[MAX_FILENAME_LEN];

/* Initialize data structures for level 1. */
int init() {
    int i;
    MINODE *mip;
    PROC *p;

    // Initialize 2 PROCs -- PO with uid=0, P1 with uid=1, all cwd=NULL
    for (i=0; i<NPROC; ++i) {
        p = &proc[i];  // let p point at proc[i]
        p->uid = p->pid = i;
        p->cwd = NULL;
        p->status = FREE;
    }

    // Initialize entries of MINODE minode[64]; all with refCount=0
    for (i=0; i<NMINODE; i++) {
        mip = &minode[i];
        mip->dev = mip->dirty = mip->ino = mip->mounted = mip->refCount = 0;
        mip->mptr = NULL;
    }

    // Set MINODE *root to 0/NULL
    root = NULL;
}

int open_dev(char *device) {
    fd = open(device, O_RDWR);   // global fd set
    if (fd < 0) {
        printf("open %s failed\n", device);
        exit(1);
    }
}

/* Validate that we're working with FS. If so, record number of blocks/inodes as globals. */
int super(char *buf) {
    // read SUPER block
    get_block(fd, 1, buf);  
    SUPER *sp = (SUPER *)buf;

    // check for EXT2 magic number to verify it's an EXT2 FS
    printf("s_magic = %x\n", sp->s_magic);
    if (sp->s_magic != 0xEF53) {
        printf("NOT an EXT2 FS\n");
        exit(1);
    }

    // record nblocks, ninodes as globals
    nblocks = sp->s_blocks_count;
    ninodes = sp->s_inodes_count;
}

/* Read group descriptor block and record block bitmap, inode bitmap, and inode start block as globals. */
int gd(char *buf) {
    // read GROUP DESCRIPTOR block
    get_block(fd, 2, buf);  
    GD *gp = (GD *)buf;

    if (!gp) {
        printf("No GROUP DESCRIPTOR block!\n");
        exit(1);
    }

    // record bamp, imap, inodes_start as globals;
    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;
}

/* Initially PO and P1 as pointing to root inode (cached minode). Set running to P0. */
int init_start_proc() {
    printf("Creating PO as running process\n");
    running = &proc[0];
    running->status = READY;
    // Let cwd of both P0 and P1 point at the root minode (refCount=3)
    proc[0].cwd = iget(dev, 2); 
    proc[1].cwd = iget(dev, 2);
}

/* mount root file system, establish / and CWDs */
int mount_root(char *disk) {
    char buf[BLKSIZE];
    
    printf("mount_root()\n");

    // open device for RW (get a file descriptor as dev for the opened device)
    super(buf);
    gd(buf);
    
    root = iget(dev, 2);    /* get root inode */
    printf("root refCount = %d\n", root->refCount);

    init_start_proc();
}