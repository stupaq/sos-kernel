#include <debug.h>

void debug_print_root_content(fs_node_t* fs_root) {
	dirent_t* node = 0;
	for (int i = 0; (node = readdir(fs_root, i)) != 0; i++) {
		fs_node_t* fsnode = finddir(fs_root, node->name);
		kprintf("\t%.20s\t0x%x\t%d\n", node->name, fsnode->flags,
				fsnode->length);
	}
}

#include <lock.h>

static spinlock_t lock = SPINLOCK_UNLOCKED;

// TODO: change those awful loops to something more polite
static int test_thread(uint32_t arg) {
	for (;;) {
		spinlock_lock(&lock);
		kprintf(">>> thread %d\n", arg);
		for (int i = 0; i < 1000000; i++)
			;
		kprintf("thread %d >>>\n", arg);
		spinlock_unlock(&lock);
		for (int i = 0; i < 1000000; i++)
			;
	}
	return 69;
}

#include <sched/fork.h>

void debug_run_threads(uint32_t num) {
	for (uint32_t i = 1; i <= num; i++)
		exec_thread((int(*)(void*)) &test_thread, (uint32_t*) i);
}
