.PHONY: all clean run rmdir rmdisk

all:
	make -C boot
	make -C kernel
	dd if=boot/fat12boot.bin of=disk.img conv=notrunc
	sudo mkdir /mnt/a
	sudo mount disk.img /mnt/a
	sudo cp kernel/kernel.bin /mnt/a
	sudo umount /mnt/a
	sudo rmdir /mnt/a

clean:
	make -C boot clean
	make -C kernel clean 

run:
	bochs -f bochsrc.bxrc -q

rmdir:
	sudo rmdir /mnt/a

rmdisk:
	sudo umount /mnt/a