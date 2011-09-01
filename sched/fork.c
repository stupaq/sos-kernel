#include <sched/fork.h>
#include <sched/sched.h>
#include <sched/thread.h>
#include <sched/task.h>
#include <mm/vmm.h>
#include <fs/fs.h>
#include <kernel/elf.h>

extern task_t* current_task;
extern thread_t* current_thread;
extern thread_t* kernel_thread;

extern uint32_t next_tid;

extern page_directory_t* current_directory;

int32_t fork() {

	if (kernel_thread == current_thread)
		panic("SCHED: no forking from kernel thread.");

	// TODO: change to kernel locking
	asm volatile("cli");

	// we will fork current thread and clone current task address space
	thread_t* new_thread = kmalloc(sizeof(thread_t));
	new_thread->tid = next_tid++;
	new_thread->state = THREAD_RUNNING;
	new_thread->esp0 = current_thread->esp0;
	new_thread->stack_size = current_thread->stack_size;

	page_directory_t* page_dir = clone_directory(current_directory);

	// NOTE: this is allocated in kernel address space so shouldn't be copied,
	// take care of properly (fully) linking kernel page tables
	task_t* new_task = create_task(new_thread, page_dir);

	// NOTE: we copy whole address space -- it means that we copy the
	// new stack (no in case when forking from kernel)

	// add to run queue
	schedule_add_task(new_task);

	// after this will be entry point
	save_thread_state(new_thread);
	// NEW TASK ENTRY POINT

	// decide if we're in child or parent
	if (current_thread == new_thread) {
		// we're in child
		return 0;
	} else {
		// we're in parent

		// TODO: change to kernel locking
		asm volatile("sti");

		// by convention return child's pid
		return new_task->pid;
	}
}

uint32_t exec_elf(const char* name) {
	fs_node_t* file = finddir(fs_root, name);
	open(file, 1, 0);

	// disable interrupts
	asm volatile("cli");

	// save directory and create one for new task
	page_directory_t* old_dir = current_directory;
	page_directory_t* new_dir = clone_directory(old_dir);

	// switch to new directory (this is where we set up process)
	switch_page_directory(new_dir);

	// load elf and get entry point
	uint32_t entry = load_elf_binary(file);

	// setup stack
	uint32_t stack = (uint32_t) allocate_stack(THREAD_STACK_SIZE);

	// create scheduling structures
	thread_t* new_thread = create_thread((int(*)(void*)) entry, 0,
			(uint32_t*) stack, THREAD_STACK_SIZE);
	task_t* new_task = create_task(new_thread, new_dir);
	schedule_add_task(new_task);

	// switch back to parents directory
	switch_page_directory(old_dir);

	// close executable image
	close(file);

	// reenable interrupts
	asm volatile("sti");

	// like in fork return child pid
	return new_task->pid;
}

uint32_t exec_thread(int(*fn)(void*), void* arg) {
	thread_t * new_thread = create_thread(fn, arg,
			allocate_stack(THREAD_STACK_SIZE), THREAD_STACK_SIZE);
	task_add_thread(current_task, new_thread);
	return new_thread->tid;
}
