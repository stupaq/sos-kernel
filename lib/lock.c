#include <lock.h>

static uint32_t atomic_test_and_set(volatile spinlock_t *lock) {
	spinlock_t value = SPINLOCK_LOCKED;
	asm volatile("lock xchgl %0, %1"
			: "=q" (value), "=m" (*lock)
			: "0" (value));
	return value;
}

void spinlock_lock(spinlock_t *lock) {
	while (atomic_test_and_set(lock) == SPINLOCK_LOCKED)
		;
}

void spinlock_unlock(spinlock_t *lock) {
	*lock = SPINLOCK_UNLOCKED;
}

uint8_t spinlock_trylock(spinlock_t *lock) {
	return atomic_test_and_set(lock) == SPINLOCK_UNLOCKED;
}
