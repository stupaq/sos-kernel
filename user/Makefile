CC=gcc -I ../kernel/include
AS=nasm
LD=ld.bfd

CFLAGS=-nostdlib -fno-builtin -m32 -Wall -Wextra -std=gnu99 -pedantic
LDFLAGS=-T user.ld
ASFLAGS=-f elf

TARGETS=init

all: $(TARGETS)

$(TARGETS): %: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) -Wl,$(LDFLAGS) $< -o $@

clean:
	@echo Cleaning
	@rm -f *.o $(TARGETS)
