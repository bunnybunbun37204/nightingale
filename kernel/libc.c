
/*
 * kformat -> kprintf
 *
 * letters  -- >  print letters
 * \char    -- >  print char
 * &colors  -- >  change console color to <colors>
 *  e.g. &70 -> change to LIGHT_GREY, BLACK (fg, bg)
 * %format  -- >  format in data
 *  e.g. %d  -> format int in from parameters (simulate printf)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <kernel/tty.h>

void _print_str(char *buf) {
    size_t i = 0;
    while (buf[i])
        putchar(buf[i++]);
}

int _print_unsigned(unsigned int value, int base) {
    const char *format = "0123456789ABCDEF";
    char buf[33];
    buf[32] = '\0';
    size_t buf_ix = 31;

    do {
        buf[buf_ix] = format[value % base];
        value /= base;
        buf_ix -= 1;
    } while (value > 0);
    _print_str(buf + buf_ix + 1);

    return 0;
}

int _print_signed(int value, int base) {
    const char *format = "0123456789ABCDEF";
    char buf[33];
    buf[32] = '\0';
    size_t buf_ix = 31;
    bool negative = false;

    if (value < 0) {
        value = -value;
        negative = true;
    }
    do {
        buf[buf_ix] = format[value % base];
        value /= base;
        buf_ix -= 1;
    } while (value > 0);
    if (negative) {
        *(buf + buf_ix) = '-';
        _print_str(buf + buf_ix);
    } else {
        _print_str(buf + buf_ix + 1);
    }
    return 0;
}

int _print_unsigned64(unsigned long long value, int base) {
    const char *format = "0123456789ABCDEF";
    char buf[65];
    buf[65] = '\0';
    size_t buf_ix = 64;

    do {
        buf[buf_ix] = format[value % base];
        value /= base;
        buf_ix -= 1;
    } while (value > 0);
    _print_str(buf + buf_ix + 1);

    return 0;
}

int _print_signed64(long long value, int base) {
    const char *format = "0123456789ABCDEF";
    char buf[65];
    buf[65] = '\0';
    size_t buf_ix = 64;
    bool negative = false;

    if (value < 0) {
        value = -value;
        negative = true;
    }
    do {
        buf[buf_ix] = format[value % base];
        value /= base;
        buf_ix -= 1;
    } while (value > 0);
    if (negative) {
        *(buf + buf_ix) = '-';
        _print_str(buf + buf_ix);
    } else {
        _print_str(buf + buf_ix + 1);
    }
    return 0;
}

int printf(const char *format, ...) {
    size_t i;
    int32_t j;
    char *s;
    va_list args;

    va_start(args, format);

    size_t len = strlen(format);
    for (i=0; i < len; i++) {
        if (format[i] != '%' && format[i] != '&') {
            putchar(format[i]);
        /* } else if (format[i] == '&') {
            set_text_color((vga_color)format[i+2] - '0', (vga_color) format[i+1] - '0');
            i += 2;
        */
        } else if (format[i] == '%') {
            switch (format[i+1]) {
                case 'c' : 
                    j = va_arg(args, int);
                    putchar(j);
                    break;
                case 'o':
                    j = va_arg(args, int);
                    _print_unsigned(j, 8);
                    break;
                case 'i' :
                    j = va_arg(args, int);
                    _print_signed(j, 10);
                    break;
                case 'u':
                    j = va_arg(args, int);
                    _print_unsigned(j, 10);
                    break;
                case 'x' :
                    j = va_arg(args, int);
                    _print_unsigned(j, 16);
                    break;
                case 's':
                    s = va_arg(args, char *);
                    _print_str(s);
                    break;
                case 'l':
                    switch (format[i+2]) {
                        case 'o':
                            j = va_arg(args, long long);
                            _print_unsigned64(j, 8);
                            break;
                        case 'i' :
                            j = va_arg(args, long long);
                            _print_signed64(j, 10);
                            break;
                        case 'u':
                            j = va_arg(args, long long);
                            _print_unsigned64(j, 10);
                            break;
                        case 'x' :
                            j = va_arg(args, long long);
                            _print_unsigned64(j, 16);
                            break;
                    }
                    i += 1;
                    break;
                case '%':
                    putchar('%');
                    break;
            }
            i += 1;
        }
    }
    return i;
}

int putchar(int value) {
    char c = (char)value;
    terminal_write(&c, sizeof(c));
    return value;
}

int puts(const char *string) {
    return printf("%s\n", string);
}

__attribute__((__noreturn__))
void abort() {
    printf("kernel: panic: abort()\n");

    __asm__ ("cli");
    for (;;)
        __asm__ ("hlt");
    __builtin_unreachable();
}

int memcmp(const void *ax, const void *bx, size_t size) {
    const uint8_t *a = (const uint8_t *)ax;
    const uint8_t *b = (const uint8_t *)bx;

    for (size_t i=0; i<size; i++) {
        if (a[i] > b[i])
            return 1;
        else if (a[i] < b[i])
            return -1;
    }
    return 0;
}

void *memcpy(void *restrict destx, const void *restrict srcx, size_t size) {
    uint8_t *dest = (uint8_t *)destx;
    const uint8_t *src = (const uint8_t *)srcx;

    for (size_t i=0; i<size; i++)
        dest[i] = src[i];

    return destx;
}

void *memmove(void *destx, const void *srcx, size_t len) {
    uint8_t *dest = (uint8_t *)destx;
    const uint8_t *src = (const uint8_t *)srcx;

    for (size_t i=0; i<len; i++)
        dest[i] = src[i];

    return destx;
}

void *memset(void *destx, int c, size_t size) {
    uint8_t *dest = (uint8_t *)destx;

    for (size_t i=0; i<size; i++) {
        dest[i] = (uint8_t)c;
    }
    return destx;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len++]);
    return len - 1;
}


