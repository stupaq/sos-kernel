#ifndef MONITOR_H
#define MONITOR_H

#include <common.h>

// Clear the screen to all black.
void init_monitor();

// Write a single character out to the screen.
void monitor_put(char c);

// Output a null-terminated ASCII string to the monitor.
void monitor_write(char* c);

// Output a hex value to the monitor.
void monitor_write_hex(uint32_t n);

// Output a decimal value to the monitor.
void monitor_write_dec(uint32_t n);

#endif // MONITOR_H
