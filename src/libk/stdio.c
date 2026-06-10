#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "drivers/serial.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

// Helper: Converts an unsigned integer to a string safely
void utoa_internal(uint64_t value, int base, char* buffer, bool uppercase) {
    char temp[65];
    int i = 0;
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    while (value > 0) {
        temp[i++] = digits[value % base];
        value /= base;
    }
    
    // Reverse into destination buffer
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// Helper: Converts a signed integer to a string safely
void itoa_internal(int64_t value, int base, char* buffer) {
    uint64_t uvalue = value;
    if (value < 0 && base == 10) {
        *buffer++ = '-';
        uvalue = (uint64_t)(-value);
    }
    utoa_internal(uvalue, base, buffer, false);
}

// Fixed and improved main vprintf function
static int vprintf_internal(const char* format, va_list args, int (*output_func)(unsigned char)) {
    int count = 0;

    while (*format) {
        if (*format == '%') {
            format++; // Move past '%'

            switch (*format) {
                case '\0': {
                    // Handle trailing '%'. Print it and exit safely.
                    output_func('%');
                    count++;
                    return count; 
                }
                case '%': {
                    output_func('%');
                    count++;
                    break;
                }
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    char buffer[32];
                    itoa_internal(num, 10, buffer);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    utoa_internal(num, 10, buffer, false);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    utoa_internal(num, 16, buffer, false);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    utoa_internal(num, 16, buffer, true);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                case 'o': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    utoa_internal(num, 8, buffer, false);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                case '#': {
                    format++; // Move past '#'
                    if (*format == 'x' || *format == 'X') {
                        unsigned int num = va_arg(args, unsigned int);
                        char buffer[32];
                        bool upper = (*format == 'X');
                        
                        output_func('0');
                        output_func(upper ? 'X' : 'x');
                        count += 2;

                        utoa_internal(num, 16, buffer, upper);
                        for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    } else {
                        // Fallback fallback if unknown modifier used with '#'
                        output_func('%');
                        output_func('#');
                        output_func(*format);
                        count += 3;
                    }
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    const char* null = "(null)";
                    if (!str) str = null;
                    while (*str) {
                        output_func(*str++);
                        count++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    output_func(c);
                    count++;
                    break;
                }
                case 'p': {
                    uintptr_t ptr = (uintptr_t)va_arg(args, void*);
                    char buffer[32];
                    output_func('0');
                    output_func('x');
                    count += 2;
                    utoa_internal(ptr, 16, buffer, false);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                case 'n': {
                    int* n = va_arg(args, int*);
                    if (n) *n = count;
                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    
                    if (num < 0) {
                        output_func('-');
                        count++;
                        num = -num;
                    }

                    // Extract integer part and fraction part safely
                    uint64_t int_part = (uint64_t)num;
                    double frac_part = num - (double)int_part;

                    char buffer[64];
                    utoa_internal(int_part, 10, buffer, false);
                    for (char* p = buffer; *p; p++) { output_func(*p); count++; }

                    output_func('.');
                    count++;

                    // Print exactly 6 digits of precision, tracking zeroes correctly
                    for (int i = 0; i < 6; i++) {
                        frac_part *= 10.0;
                        int digit = (int)frac_part;
                        output_func('0' + digit);
                        count++;
                        frac_part -= digit;
                    }
                    break;
                }
                case 'e':
                case 'E': {
                    double num = va_arg(args, double);
                    bool upper = (*format == 'E');
                    int exponent = 0;

                    if (num < 0) {
                        output_func('-');
                        count++;
                        num = -num;
                    }

                    if (num != 0.0) {
                        while (num >= 10.0) {
                            num /= 10.0;
                            exponent++;
                        }
                        while (num < 1.0) {
                            num *= 10.0;
                            exponent--;
                        }
                    }

                    // Print the mantissa component
                    uint64_t int_part = (uint64_t)num;
                    double frac_part = num - (double)int_part;

                    output_func('0' + (int)int_part);
                    output_func('.');
                    count += 2;

                    for (int i = 0; i < 6; i++) {
                        frac_part *= 10.0;
                        int digit = (int)frac_part;
                        output_func('0' + digit);
                        count++;
                        frac_part -= digit;
                    }

                    // Print exponent format: e+00 / E-02
                    output_func(upper ? 'E' : 'e');
                    output_func(exponent >= 0 ? '+' : '-');
                    count += 2;

                    if (exponent < 0) exponent = -exponent;

                    char exp_buffer[16];
                    utoa_internal(exponent, 10, exp_buffer, false);
                    
                    // Zero-pad small scientific exponents up to 2 places (e.g., e+05)
                    if (exponent < 10) {
                        output_func('0');
                        count++;
                    }
                    for (char* p = exp_buffer; *p; p++) { output_func(*p); count++; }
                    break;
                }
                default:
                    // Unknown specifier, print it literally
                    output_func('%');
                    output_func(*format);
                    count += 2;
                    break;
            }
            format++; // Done processing token, advance past it
        } else {
            output_func(*format++);
            count++;
        }
    }
    return count;
}

int printf_serial(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf_internal(format, args, putchar_serial);
    va_end(args);
    return result;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf_internal(format, args, putchar);
    va_end(args);
    return result;
}