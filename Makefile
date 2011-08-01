CSOURCES=$(shell find -maxdepth 4 -name *.c)
COBJECTS=$(patsubst %.c, %.o, $(CSOURCES))
SSOURCES=$(shell find -maxdepth 4 -name *.s)
SOBJECTS=$(patsubst %.s, %.o, $(SSOURCES))

CC=gcc -I include
AS=nasm
LD=ld.bfd

CFLAGS=-nostdlib -fno-builtin -m32 -Wall -Wextra -std=gnu99 -pedantic
LDFLAGS=-melf_i386 -Tlink.ld
ASFLAGS=-felf

all: $(COBJECTS) $(SOBJECTS) link

clean:
	@echo Removing object files
	@-rm $(COBJECTS) $(SOBJECTS) kernel.bin 2>/dev/null
	@(cd ../utils; make clean)

link:
	@echo Linking
	@$(LD) $(LDFLAGS) -o kernel.bin $(SOBJECTS) $(COBJECTS)

.s.o:
	@echo Assembling $<
	@$(AS) $(ASFLAGS) $<

.c.o:
	@echo Compiling $<
	@$(CC) $(CFLAGS) -o $@ -c $<

utils:
	@(cd ../utils; make)

dep:
	@echo Computing dependencies
	@sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	@(for i in $(CSOURCES); do $(CPP) -M $$i; done) >> tmp_make
	@mv tmp_make Makefile

### Dependencies
initrd.o: fs/initrd.c include/initrd.h include/common.h include/kprintf.h \
 include/panic.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h include/fs.h
fs.o: fs/fs.c include/fs.h include/common.h include/kprintf.h \
 include/panic.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h
lock.o: kernel/lock.c include/lock.h include/common.h include/kprintf.h \
 include/panic.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h include/thread.h include/scheduler.h include/monitor.h
elf.o: kernel/elf.c include/elf.h include/common.h include/kprintf.h \
 include/panic.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h
scheduler.o: kernel/scheduler.c include/scheduler.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/pmm.h \
 include/multiboot.h include/string.h include/thread.h
panic.o: kernel/panic.c include/panic.h include/common.h \
 include/kprintf.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h include/elf.h
keyboard.o: kernel/keyboard.c include/keyboard.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/pmm.h \
 include/multiboot.h include/string.h include/idt.h
idt.o: kernel/idt.c include/common.h include/kprintf.h include/panic.h \
 include/heap.h include/pmm.h include/multiboot.h include/string.h \
 include/idt.h
gdt.o: kernel/gdt.c include/common.h include/kprintf.h include/panic.h \
 include/heap.h include/pmm.h include/multiboot.h include/string.h \
 include/gdt.h
timer.o: kernel/timer.c include/common.h include/kprintf.h \
 include/panic.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h include/timer.h include/idt.h include/scheduler.h \
 include/thread.h
thread.o: kernel/thread.c include/thread.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/pmm.h \
 include/multiboot.h include/string.h include/scheduler.h
vmm.o: mm/vmm.c include/vmm.h include/common.h include/kprintf.h \
 include/panic.h include/heap.h include/pmm.h include/multiboot.h \
 include/string.h include/idt.h
pmm.o: mm/pmm.c include/pmm.h include/multiboot.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/string.h \
 include/vmm.h
heap.o: mm/heap.c include/heap.h include/common.h include/kprintf.h \
 include/panic.h include/string.h include/pmm.h include/multiboot.h \
 include/vmm.h
string.o: lib/string.c include/string.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/pmm.h \
 include/multiboot.h
vsprintf.o: lib/vsprintf.c \
 /usr/lib/gcc/i686-pc-linux-gnu/4.6.1/include/stdarg.h include/string.h \
 include/common.h include/kprintf.h include/panic.h include/heap.h \
 include/pmm.h include/multiboot.h
common.o: lib/common.c include/common.h include/kprintf.h include/panic.h \
 include/heap.h include/pmm.h include/multiboot.h include/string.h
kprintf.o: lib/kprintf.c include/kprintf.h include/vsprintf.h \
 /usr/lib/gcc/i686-pc-linux-gnu/4.6.1/include/stdarg.h include/monitor.h \
 include/common.h include/panic.h include/heap.h include/pmm.h \
 include/multiboot.h include/string.h
monitor.o: lib/monitor.c include/monitor.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/pmm.h \
 include/multiboot.h include/string.h
main.o: init/main.c include/multiboot.h include/common.h \
 include/kprintf.h include/panic.h include/heap.h include/pmm.h \
 include/string.h include/monitor.h include/gdt.h include/idt.h \
 include/timer.h include/elf.h include/vmm.h include/thread.h \
 include/lock.h include/scheduler.h include/keyboard.h include/initrd.h \
 include/fs.h
