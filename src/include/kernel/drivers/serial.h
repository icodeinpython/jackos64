#pragma once

#include <stdint.h>

#define PORT 0x3F8  // COM1

void serial_init();
char read_serial();
void write_serial(unsigned char a);
