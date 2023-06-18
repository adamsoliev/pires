# Makefile for building, running and debugging assembly code on qemu-riscv64

# Define the name of your assembly file (without the .s extension)
ASSEMBLY_FILE = print_simple

TARGET = $(ASSEMBLY_FILE)

QEMU = qemu-riscv64

CC = riscv64-unknown-linux-gnu-gcc
CFLAGS = -march=rv64g -mabi=lp64d

all: $(TARGET)

$(TARGET): $(ASSEMBLY_FILE).s
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	$(QEMU) -L /opt/riscv/sysroot ./$<

debug: $(TARGET)
	$(QEMU) -L /opt/riscv/sysroot -g 1234 -S ./$< &
	riscv64-unknown-linux-gnu-gdb -ex "target remote:1234" $<

# riscv64-unknown-linux-gnu-gdb -batch -ex 'disassemble load_gp' $< 
disassemble: $(TARGET)
	riscv64-unknown-linux-gnu-objdump -d --disassembler-color=on $<

headers: $(TARGET)
	riscv64-unknown-linux-gnu-readelf -e -W $< 

clean:
	rm -f $(TARGET)


.PHONY: all run debug clean 