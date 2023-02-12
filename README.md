# Small monolithic kernel with custom bootloader
My personal project from 2019-2021.
### Features
- [x] FAT12 bootloader
- [x] PIC interrupt handling
- [x] Kernel space kmalloc
- [x] Kernel space vmalloc
- [x] Kernel threads
- [x] UART logging
- [ ] Userspace
- [ ] System calls

### Dependencies
- dd
- make
- nasm
- [i386 gcc cross compiler](https://wiki.osdev.org/GCC_Cross-Compiler)
- bochs

### Build
First proper bochs configuration has to be created. It should load `disk.img` from project root as 3.5inch disk A. Then run `make && make run`.
