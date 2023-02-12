.PHONY: all clean

all: fat12boot.bin
fat12boot.bin: fat12boot.asm boot.asm OpenA20.asm BIOSparamblock.asm
	nasm fat12boot.asm -f bin -o fat12boot.bin
clean:
	rm fat12boot.bin