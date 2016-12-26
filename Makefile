KFLAGS=-ffreestanding -std=c99
KINCLUDE=-Ikernel

_CORE_MODULE = print hal gdt idt pit pic keyboard debug physical string page_table_entry page_directory_entry virtual stdin_buffer ata_pio file_system kmalloc
_LIB_MODULE = string_tokenizer

# Note: $(patsubst pattern,replacement,string)
CORE_OBJS = $(patsubst %,kernel/core/%.o,$(_CORE_MODULE))
LIB_OBJS = $(patsubst %,kernel/lib/%.o,$(_LIB_MODULE))

# Empty implicit suffix rule
.SUFFIXES:

all: boot hello_world
	cat bootloader.bin enter_protected_mode.bin kernel/hello_world.bin > tio_os.img

boot: boot/enter_protected_mode.asm boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o bootloader.bin
	nasm -f bin boot/enter_protected_mode.asm -o enter_protected_mode.bin

hello_world: kernel/hello_world.c kernel/hello_world.ld $(CORE_OBJS) $(LIB_OBJS)
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/hello_world.c -o kernel/hello_world.o $(KINCLUDE)
	i686-elf-ld -T kernel/hello_world.ld --oformat=binary -nostdlib -o kernel/hello_world.bin kernel/hello_world.o $(CORE_OBJS) $(LIB_OBJS)

# Compile the core objects
# Note $@ := left hand side of ':'
#      $< := first item in the dependency list
# Note % is pattern matching syntax, similar to LIKE in SQL.
kernel/core/%.o: kernel/core/%.c kernel/core/%.h
	i686-elf-gcc $(KFLAGS) $(KINCLUDE) -c $< -o $@

kernel/lib/%.o: kernel/lib/%.c kernel/lib/%.h
	i686-elf-gcc $(KFLAGS) $(KINCLUDE) -c $< -o $@

clean:
	rm $(CORE_OBJS) $(LIB_OBJS) kernel/*.o kernel/*.bin *.bin *.img

run:
	qemu-img resize tio_os.img +10M
	qemu-system-x86_64 -d guest_errors -hda tio_os.img -m 256