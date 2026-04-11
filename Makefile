# Sega Dreamcast Makefile for HelloWorld

# The name of your final executable
TARGET = $(notdir $(CURDIR)).elf

# Romdisk configuration
KOS_ROMDISK_DIR = romdisk

# The list of compiled object files (add more .o files here if you add more .c files)
OBJS = main.o romdisk.o

# The default target executed when you just type 'make'
all: rm-elf $(TARGET)

# Include the standard KallistiOS rules and paths
include $(KOS_BASE)/Makefile.rules

clean: rm-elf
	-rm -f $(OBJS) romdisk.img romdisk.o

rm-elf:
	-rm -f $(TARGET)

# Link the objects to create the final ELF file
$(TARGET): $(OBJS) romdisk.img
	kos-cc -o $(TARGET) $(OBJS) $(OBJEXTRA)
