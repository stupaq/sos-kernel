#ifndef LOCK_H
#define LOCK_H

#include <common.h>

#define SPINLOCK_LOCKED 0
#define SPINLOCK_UNLOCKED 1

typedef volatile uint32_t spinlock_t;

void spinlock_lock(spinlock_t *lock);

void spinlock_unlock(spinlock_t *lock);

uint8_t spinlock_trylock(spinlock_t *lock);

#endif
