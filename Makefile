CSOURCES=$(shell find -maxdepth 4 -name *.c)
COBJECTS=$(patsubst %.c, %.o, $(CSOURCES))
SSOURCES=$(shell find -maxdepth 4 -name *.s)
SOBJECTS=$(patsubst %.s, %.o, $(SSOURCES))

CC=gcc -I include
AS=nasm
LD=ld.bfd

CFLAGS=-nostdlib -fno-builtin -m32 -Wall -Wextra -std=gnu99 -pedantic
LDFLAGS=-T kernel.ld
ASFLAGS=-felf

all: $(COBJECTS) $(SOBJECTS) link tools user update

clean:
	@echo Removing object files
	@-rm $(COBJECTS) $(SOBJECTS) kernel.bin 2>/dev/null
	@(cd ../tools; make clean)
	@(cd ../user; make clean)

link:
	@echo Linking
	@$(LD) $(LDFLAGS) -o kernel.bin $(SOBJECTS) $(COBJECTS)

.s.o:
	@echo Assembling $<
	@$(AS) $(ASFLAGS) $<

.c.o:
	@echo Compiling $<
	@$(CC) $(CFLAGS) -o $@ -c $<

tools:
	@(cd ../tools; make)

user:
	@(cd ../user; make)

update:
	@echo Updating image
	@(cd ../; ./update_floppy.sh)

dep:
	@echo Computing dependencies
	@sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	@(for i in $(CSOURCES); do $(CPP) -M $$i; done) >> tmp_make
	@mv tmp_make Makefile

### Dependencies
initrd.o: fs/initrd.c include/fs/initrd.h include/fs/fs.h \
 include/common.h include/kernel/panic.h include/mm/kheap.h \
 include/mm/pmm.h include/boot/multiboot.h include/mm/layout.h \
 include/kprintf.h include/string.h
fs.o: fs/fs.c include/fs/fs.h include/common.h include/kernel/panic.h \
 include/mm/kheap.h include/mm/pmm.h include/boot/multiboot.h \
 include/mm/layout.h include/kprintf.h include/string.h
elf.o: kernel/elf.c include/kernel/elf.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/kernel/elf-fn.h include/fs/fs.h \
 include/mm/vmm.h
panic.o: kernel/panic.c include/kernel/panic.h include/kernel/elf.h \
 include/common.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/kernel/elf-fn.h include/fs/fs.h
idt.o: kernel/idt.c include/kernel/idt.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h
kbd.o: kernel/kbd.c include/kernel/kbd.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/kernel/idt.h
gdt.o: kernel/gdt.c include/kernel/gdt.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h
timer.o: kernel/timer.c include/kernel/timer.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/kernel/idt.h include/sched/sched.h \
 include/sched/thread.h include/sched/task.h include/list.h \
 include/mm/vmm.h
kheap.o: mm/kheap.c include/mm/kheap.h include/common.h \
 include/kernel/panic.h include/kprintf.h include/string.h \
 include/mm/pmm.h include/boot/multiboot.h include/mm/layout.h \
 include/mm/vmm.h
vmm.o: mm/vmm.c include/mm/vmm.h include/common.h include/kernel/panic.h \
 include/mm/kheap.h include/mm/pmm.h include/boot/multiboot.h \
 include/mm/layout.h include/kprintf.h include/string.h \
 include/kernel/idt.h
pmm.o: mm/pmm.c include/mm/pmm.h include/boot/multiboot.h \
 include/common.h include/kernel/panic.h include/mm/kheap.h \
 include/kprintf.h include/string.h include/mm/layout.h include/mm/vmm.h
lock.o: lib/lock.c include/lock.h include/common.h include/kernel/panic.h \
 include/mm/kheap.h include/mm/pmm.h include/boot/multiboot.h \
 include/mm/layout.h include/kprintf.h include/string.h
string.o: lib/string.c include/string.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h
vsprintf.o: lib/vsprintf.c include/vsprintf.h include/stdarg.h \
 include/string.h include/common.h include/kernel/panic.h \
 include/mm/kheap.h include/mm/pmm.h include/boot/multiboot.h \
 include/mm/layout.h include/kprintf.h
common.o: lib/common.c include/common.h include/kernel/panic.h \
 include/mm/kheap.h include/mm/pmm.h include/boot/multiboot.h \
 include/mm/layout.h include/kprintf.h include/string.h
list.o: lib/list.c include/list.h include/common.h include/kernel/panic.h \
 include/mm/kheap.h include/mm/pmm.h include/boot/multiboot.h \
 include/mm/layout.h include/kprintf.h include/string.h
kprintf.o: lib/kprintf.c include/kprintf.h include/vsprintf.h \
 include/stdarg.h include/monitor.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/string.h
monitor.o: lib/monitor.c include/monitor.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h
main.o: init/main.c include/boot/multiboot.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/mm/layout.h include/kprintf.h include/string.h \
 include/kernel/gdt.h include/kernel/idt.h include/kernel/timer.h \
 include/kernel/elf.h include/kernel/elf-fn.h include/fs/fs.h \
 include/kernel/kbd.h include/sched/sched.h include/sched/thread.h \
 include/sched/task.h include/list.h include/mm/vmm.h \
 include/sched/fork.h include/fs/initrd.h include/monitor.h \
 include/debug.h
debug.o: init/debug.c include/debug.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/fs/fs.h include/lock.h include/sched/fork.h \
 include/kernel/idt.h include/mm/vmm.h include/sched/task.h \
 include/list.h include/sched/thread.h
sched.o: sched/sched.c include/sched/sched.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/sched/thread.h include/sched/task.h \
 include/list.h include/mm/vmm.h
thread.o: sched/thread.c include/sched/thread.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/mm/vmm.h
fork.o: sched/fork.c include/sched/fork.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/sched/sched.h include/sched/thread.h \
 include/sched/task.h include/list.h include/mm/vmm.h include/fs/fs.h \
 include/kernel/elf.h include/kernel/elf-fn.h
task.o: sched/task.c include/sched/task.h include/common.h \
 include/kernel/panic.h include/mm/kheap.h include/mm/pmm.h \
 include/boot/multiboot.h include/mm/layout.h include/kprintf.h \
 include/string.h include/list.h include/sched/thread.h include/mm/vmm.h
