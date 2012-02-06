all: update

update: kernel tools user
	@echo Updating image
	./update_floppy.sh

kernel:
	@(cd ./kernel; make)

tools:
	@(cd ./tools; make)

user:
	@(cd ./user; make)

CLEANUP=*.log initrd.img floppy.img
clean:
	@(cd ./kernel; make clean)
	@(cd ./tools; make clean)
	@(cd ./user; make clean)
	@echo Cleaning
	@rm -f $(CLEANUP)

.PHONY: kernel tools user clean

