all: enter_protected_mode.asm bootloader.asm hello_world
	nasm -f bin bootloader.asm -o bootloader.bin
	nasm -f bin enter_protected_mode.asm -o enter_protected_mode.bin
	cat bootloader.bin enter_protected_mode.bin kernel/hello_world.bin > tio_os.img

hello_world: kernel/hello_world.c kernel/hello_world.ld printing harware_abstraction_layer gdt idt pit pic keyboard debug physical string pagetable_entry page_directory_entry virtual stdin_buffer ata_pio file_system kmalloc
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/hello_world.c -o kernel/hello_world.o
	i686-elf-ld -T kernel/hello_world.ld --oformat=binary -nostdlib -o kernel/hello_world.bin kernel/hello_world.o kernel/print.o kernel/hal.o kernel/gdt.o kernel/idt.o kernel/pic.o kernel/pit.o kernel/keyboard.o kernel/debug.o kernel/physical.o kernel/string.o kernel/page_table_entry.o kernel/page_directory_entry.o kernel/virtual.o kernel/stdin_buffer.o kernel/ata_pio.o kernel/file_system.o kernel/kmalloc.o

printing: kernel/print.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/print.c -o kernel/print.o

debug: kernel/debug.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/debug.c -o kernel/debug.o

harware_abstraction_layer: kernel/hal.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/hal.c -o kernel/hal.o

gdt: kernel/gdt.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/gdt.c -o kernel/gdt.o

idt: kernel/idt.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/idt.c -o kernel/idt.o

pit: kernel/pit.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/pit.c -o kernel/pit.o

pic: kernel/pic.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/pic.c -o kernel/pic.o

keyboard: kernel/keyboard.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/keyboard.c -o kernel/keyboard.o

physical: kernel/physical.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/physical.c -o kernel/physical.o

string: kernel/string.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/string.c -o kernel/string.o

pagetable_entry: kernel/page_table_entry.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/page_table_entry.c -o kernel/page_table_entry.o

page_directory_entry: kernel/page_directory_entry.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/page_directory_entry.c -o kernel/page_directory_entry.o

virtual: kernel/virtual.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/virtual.c -o kernel/virtual.o

stdin_buffer: kernel/stdin_buffer.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/stdin_buffer.c -o kernel/stdin_buffer.o

ata_pio: kernel/ata_pio.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/ata_pio.c -o kernel/ata_pio.o

file_system: kernel/file_system.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/file_system.c -o kernel/file_system.o

kmalloc: kernel/kmalloc.c
	i686-elf-gcc -ffreestanding -std=c99 -c kernel/kmalloc.c -o kernel/kmalloc.o

run:
	qemu-img resize tio_os.img +10M
	qemu-system-x86_64 -d guest_errors -hda tio_os.img -m 256