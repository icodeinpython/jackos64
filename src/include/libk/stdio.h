#pragma once

#include <stdarg.h>

// serial
int putchar_serial(unsigned char c);
int puts_serial(const char* str);
int printf_serial(const char* format, ...);

// terminal
int putchar(unsigned char c);
int puts(const char* str);
int printf(const char* format, ...);