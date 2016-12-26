CC=i686-elf-gcc
CFLAGS=-ffreestanding -std=c99

_CORE_MODULE = print hal gdt idt pit pic keyboard debug physical string page_table_entry page_directory_entry virtual stdin_buffer ata_pio file_system kmalloc

# Note: $(patsubst pattern,replacement,string)
CORE_OBJS = $(patsubst %,kernel/core/%.o,$(_CORE_MODULE))
CORE_HEADERS = $(patsubst %,kernel/core/%.h,%(_CORE_MODULE))

all: enter_protected_mode.asm bootloader.asm hello_world
	nasm -f bin bootloader.asm -o bootloader.bin
	nasm -f bin enter_protected_mode.asm -o enter_protected_mode.bin
	cat bootloader.bin enter_protected_mode.bin kernel/hello_world.bin > tio_os.img

hello_world: kernel/hello_world.c kernel/hello_world.ld $(CORE_OBJS)
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/hello_world.c -o kernel/hello_world.o
	i686-elf-ld -T kernel/hello_world.ld --oformat=binary -nostdlib -o kernel/hello_world.bin kernel/hello_world.o $(CORE_OBJS)

# Compile the core objects
# Note $@ := left hand side of ':'
#      $< := first item in the dependency list
kernel/%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(CORE_OBJS) kernel/*.o kernel/*.bin *.bin *.img

run:
	qemu-img resize tio_os.img +10M
	qemu-system-x86_64 -d guest_errors -hda tio_os.img -m 256