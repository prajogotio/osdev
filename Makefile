all: enter_protected_mode.asm bootloader.asm hello_world
	nasm -f bin bootloader.asm -o bootloader.bin
	nasm -f bin enter_protected_mode.asm -o enter_protected_mode.bin
	cat bootloader.bin enter_protected_mode.bin kernel/hello_world.bin > tio_os.img
	#dd if=/dev/zero bs=512 count=3 >> tio_os.img

test: enter_protected_mode.asm bootloader.asm first_kernel.asm
	nasm -f bin bootloader.asm -o bootloader.bin
	nasm -f bin enter_protected_mode.asm -o enter_protected_mode.bin
	nasm -f bin first_kernel.asm -o first_kernel.bin
	cat bootloader.bin enter_protected_mode.bin first_kernel.bin > tio_os.img

hello_world: kernel/hello_world.c kernel/hello_world.ld printing harware_abstraction_layer gdt idt pit pic keyboard debug
	i686-elf-gcc -std=c99 -c kernel/hello_world.c -o kernel/hello_world.o
	i686-elf-ld -T kernel/hello_world.ld --oformat=binary -nostdlib -o kernel/hello_world.bin kernel/hello_world.o kernel/print.o kernel/hal.o kernel/gdt.o kernel/idt.o kernel/pic.o kernel/pit.o kernel/keyboard.o kernel/debug.o

printing: kernel/print.c
	i686-elf-gcc -std=c99 -c kernel/print.c -o kernel/print.o

debug: kernel/debug.c
	i686-elf-gcc -std=c99 -c kernel/debug.c -o kernel/debug.o

harware_abstraction_layer: kernel/hal.c
	i686-elf-gcc -std=c99 -c kernel/hal.c -o kernel/hal.o

gdt: kernel/gdt.c
	i686-elf-gcc -std=c99 -c kernel/gdt.c -o kernel/gdt.o

idt: kernel/idt.c
	i686-elf-gcc -std=c99 -c kernel/idt.c -o kernel/idt.o

pit: kernel/pit.c
	i686-elf-gcc -std=c99 -c kernel/pit.c -o kernel/pit.o

pic: kernel/pic.c
	i686-elf-gcc -std=c99 -c kernel/pic.c -o kernel/pic.o

keyboard: kernel/keyboard.c
	i686-elf-gcc -std=c99 -c kernel/keyboard.c -o kernel/keyboard.o

run:
	#qemu-img create -f raw tio_os.img 5K
	qemu-system-x86_64 -d guest_errors tio_os.img