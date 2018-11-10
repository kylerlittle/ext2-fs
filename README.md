# EXT2 File System on Virtual FD

## Description
A Linux-compatible EXT2 file system. See more information [here](https://www.eecs.wsu.edu/~cs360/proj10.html).

## Usage
Partition disk if no disk present.
```
bash scripts/creat_vdisk.sh
```
Next, ```cd``` into ```src```.
Then, run as:
```
make run
```

## Help
If ```<ext2fs/ext2_fs.h>``` header is missing, run:
```
sudo apt-get install e2fslibs-dev zlib1g-dev libssl-dev
```

## Members
- Kyler Little
- Slater Weinstock