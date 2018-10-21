# EXT2 File System on Virtual FD

## Description
A Linux-compatible EXT2 file system. See more information [here](https://www.eecs.wsu.edu/~cs360/proj10.html).

## Usage
Partition disk
```
bash scripts/creat_vdisk.sh
```
Compile desired file with gcc using ```-m32``` flag.

Run as:
```
./a.out path/to/mydisk
```

## Help
If ```<ext2fs/ext2_fs.h>``` header is missing, run:
```
sudo apt-get install e2fslibs-dev zlib1g-dev libssl-dev
```

## Members
- Kyler Little
- Slater Weinstock