CC=gcc -I ../kernel/include
CFLAGS=-Wall -Wextra -std=gnu99 -pedantic

TARGETS=make_initrd

all: $(TARGETS)

$(TARGETS): %: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@echo Cleaning
	@rm -f *.o $(TARGETS)
