#! /bin/bash
sudo mount -o loop mydisk /mnt
sudo rm -rf /mnt/*
sudo umount /mnt
rm mydisk