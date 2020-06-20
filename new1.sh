#!/bin/bash
sudo qemu-img create -f qcow2 -o backing_file=minixBackup.img minix1.img
sudo qemu-system-x86_64 -curses -drive file=minix1.img -enable-kvm -rtc base=localtime -net user,hostfwd=tcp::10022-:22 -net nic,model=virtio -m 1024M
