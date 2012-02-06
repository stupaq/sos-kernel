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

#include <kernel/idt.h>
#include <mm/vmm.h>
#include <sched/task.h>
#include <sched/thread.h>

extern page_directory_t* current_directory;
extern page_directory_t kernel_directory;
extern task_t* current_task;
extern thread_t* current_thread;

static void debug_universal_handler(registers_t* regs) {
	kprintf("Scheduler: pid: %d tid: %d\n", current_task->pid,
			current_thread->tid);
	kprintf("Current directory: 0x%.8x %s\n", current_directory->physical,
			(current_directory == &kernel_directory) ? "kernel" : "");
	kprintf("Registers: eip: 0x%x\n", regs->eip);
	kprintf("Error code: %x\n", regs->err_code);
	panic("DEBUG universal handler");
}

void debug_register_handler(uint8_t n) {
	register_interrupt_handler(n, debug_universal_handler);
}
