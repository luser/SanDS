# Makefile for framebuffer_demo1.nds
# chris.double@double.co.nz
NDSLIB_INCLUDE=$(DEVKITPRO)/libnds/include
NDSLIB_LIB=$(DEVKITPRO)/libnds/lib

all: sands.nds.gba

arm7_main.o: arm7_main.cpp
	arm-elf-g++ -g -Wall -O2 -mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer -ffast-math -mthumb-interwork -I$(NDSLIB_INCLUDE) -DARM7 -c arm7_main.cpp -oarm7_main.o

arm7.elf: arm7_main.o
	arm-elf-g++ -g -mthumb-interwork -mno-fpu -specs=ds_arm7.specs arm7_main.o -L$(NDSLIB_LIB) -lnds7 -oarm7.elf

arm7.bin: arm7.elf
	arm-elf-objcopy -O binary arm7.elf arm7.bin

arm9_main.o: arm9_main.cpp
	arm-elf-g++ -g -Wall -O2 -mcpu=arm9tdmi -mtune=arm9tdmi -fomit-frame-pointer -ffast-math -mthumb-interwork -I$(NDSLIB_INCLUDE) -DARM9 -c arm9_main.cpp -oarm9_main.o

arm9.elf: arm9_main.o
	arm-elf-g++ -g -mthumb-interwork -mno-fpu -specs=ds_arm9.specs arm9_main.o -L$(NDSLIB_LIB) -lnds9 -o arm9.elf

arm9.bin: arm9.elf
	arm-elf-objcopy -O binary arm9.elf arm9.bin

sands.nds: arm7.bin arm9.bin 
	ndstool -c sands.nds -9 arm9.bin -7 arm7.bin

sands.nds.gba: sands.nds
	dsbuild sands.nds -o sands.nds.gba

clean:
	rm -f *.bin
	rm -f *.elf
	rm -f *.o
	rm -f *~

