# # Name of the assembly file (without the file extension)
# FILENAME = hello_world

# # RISC-V compiler and flags
# CC = riscv64-unknown-linux-gnu-gcc
# #CFLAGS = -march=rv64i #-mabi=ilp32

# EMU = qemu-riscv64 

# CCGDB = riscv64-unknown-linux-gnu-gdb

# # Default target
# all: $(FILENAME)

# # Rule to compile the assembly file
# $(FILENAME): $(FILENAME).s
# 	$(CC) $(CFLAGS) -o $(FILENAME) $(FILENAME).s

# run:
# 	$(EMU) -L /home/adam/dev/riscv/sysroot ./$(FILENAME)

# # Clean rule to remove the generated executable
# clean:
# 	rm -f $(FILENAME)

# debug:
# 	$(CCGDB) -ex "target remote | $(EMU) -L /home/adam/dev/riscv/sysroot ./$(FILENAME)"


# Makefile for building and running assembly code on qemu-riscv64

# Define the name of your assembly file (without the .s extension)
ASSEMBLY_FILE = hello_world

# Define the target executable name
TARGET = $(ASSEMBLY_FILE)

# Define the path to the RISC-V toolchain and QEMU
RISCV_TOOLCHAIN_PATH = /opt/riscv/bin
QEMU = qemu-riscv64

# Define the compiler and linker flags
CC = $(RISCV_TOOLCHAIN_PATH)/riscv64-unknown-linux-gnu-gcc
LD = $(RISCV_TOOLCHAIN_PATH)/riscv64-unknown-linux-gnu-ld
CFLAGS = -march=rv64imafdc -mabi=lp64d


all: $(TARGET)

$(TARGET): $(ASSEMBLY_FILE).s
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
#$(QEMU) -L /home/adam/dev/riscv/sysroot ./$<
	$(QEMU) -L /opt/riscv/sysroot ./$<


debug: $(TARGET)
	$(QEMU) -L /home/adam/dev/riscv/sysroot -gdb tcp::1234 -S ./$< &
	$(RISCV_TOOLCHAIN_PATH)/riscv64-unknown-linux-gnu-gdb -ex "target remote localhost:1234" $<

clean:
	rm -f $(TARGET)


.PHONY: all run debug clean 