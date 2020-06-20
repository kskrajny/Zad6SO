sudo qemu-system-x86_64 -curses -drive file=minix2.img -enable-kvm -drive file=extra.img,format=raw,index=1,media=disk -rtc base=localtime -net user,hostfwd=tcp::10023-:22 -net nic,model=virtio -m 1024M

