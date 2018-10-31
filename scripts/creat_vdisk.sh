#! /bin/bash
dd if=/dev/zero of=mydisk bs=1024 count=1440
mkfs -b 1024 mydisk 1440
sudo mount -o loop mydisk /mnt
(cd /mnt; sudo mkdir dir1 dir2 dir3 dir4 dir1/dir5; sudo touch file1 file2 file3 file4 dir1/file5 dir1/dir5/file6; ls -l)
sudo umount /mnt
