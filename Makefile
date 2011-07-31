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
initrd.o: fs/initrd.c include/initrd.h include/common.h include/fs.h \
 include/common.h include/string.h include/kheap.h \
 include/ordered_array.h
fs.o: fs/fs.c include/fs.h include/common.h
sched.o: kernel/sched.c include/sched.h include/common.h include/paging.h \
 include/isr.h include/common.h include/kheap.h include/ordered_array.h \
 include/descriptors.h include/string.h
descriptors.o: kernel/descriptors.c include/common.h \
 include/descriptors.h include/isr.h include/common.h include/string.h
gdt.o: kernel/gdt.c include/common.h include/gdt.h
timer.o: kernel/timer.c include/timer.h include/common.h include/isr.h \
 include/common.h include/monitor.h include/sched.h include/paging.h
sys.o: kernel/sys.c include/sys.h include/calls.h include/monitor.h \
 include/common.h include/sched.h include/paging.h include/isr.h \
 include/common.h
isr.o: kernel/isr.c include/common.h include/isr.h include/common.h \
 include/monitor.h
calls.o: kernel/calls.c include/isr.h include/common.h include/calls.h \
 include/sys.h include/monitor.h include/common.h include/sched.h \
 include/paging.h
kheap.o: mm/kheap.c include/kheap.h include/common.h \
 include/ordered_array.h include/paging.h include/isr.h include/common.h
paging.o: mm/paging.c include/paging.h include/common.h include/isr.h \
 include/common.h include/kheap.h include/ordered_array.h \
 include/string.h include/monitor.h
string.o: lib/string.c include/string.h include/common.h
common.o: lib/common.c include/common.h include/monitor.h
ordered_array.o: lib/ordered_array.c include/ordered_array.h \
 include/common.h include/kheap.h include/string.h
monitor.o: lib/monitor.c include/monitor.h include/common.h
kmain.o: init/kmain.c include/monitor.h include/common.h \
 include/descriptors.h include/timer.h include/paging.h include/isr.h \
 include/common.h include/multiboot.h include/fs.h include/initrd.h \
 include/fs.h include/sched.h include/sys.h include/calls.h
