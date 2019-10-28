
#include <ng/basic.h>
#include <ng/panic.h>
#include <ng/string.h>

bool isalnum(char c) {
        return ((c >= '0') && (c <= '9')) ||
               ((c >= 'A') && (c <= 'Z')) ||
               ((c >= 'a') && (c <= 'z'));
}

bool isalpha(char c) {
        return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

bool islower(char c) {
        return ((c >= 'a') && (c <= 'z'));
}

bool isupper(char c) {
        return ((c >= 'A') && (c <= 'Z'));
}

bool isdigit(char c) {
        return ((c >= '0') && (c <= '9'));
}

bool isxdigit(char c) {
        return ((c >= '0') && (c <= '9')) ||
               ((c >= 'a') && (c <= 'f')) ||
               ((c >= 'A') && (c <= 'F'));
}

bool iscntrl(char c) {
        return (c <= 31 || c == 127);
}

bool isspace(char c) {
        return (c >= 9 && c <= 13) || c == ' '; // c in "\t\n\v\f\r "
}

bool isblank(char c) {
        return (c == 9 || c == ' ');
}

bool isprint(char c) {
        return (c > 31 && c < 127);
}

bool ispunct(char c) {
        return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') ||
               (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
}

char *strcpy(char *dest, const char *src) {
        while (*src != 0) {
                *dest++ = *src++;
        }
        *dest = *src; // copy the \0

        return dest;
}

char *strncpy(char *dest, const char *src, size_t count) {
        int i;
        for (i = 0; i < count && *src != 0; i++) {
                *dest++ = *src++;
        }
        if (i < count) {
                *dest = *src; // copy the \0 if there is room left
        }
        return dest;
}

size_t strlen(const char *s) {
        size_t i = 0;
        while (*s++ != 0) {
                i++;
        }
        return i;
}

int strcmp(const char *a, const char *b) {
        while (*a == *b) {
                if (*a == 0) {
                        return 0;
                }
                a++;
                b++;
        }
        return *b - *a; // test!
}

int strncmp(const char *a, const char *b, size_t count) {
        for (size_t i = 0; i < count; i++) {
                if (*a == *b) {
                        a++, b++;
                        continue;
                }
                return *b - *a;
        }
        return 0;
}

char *strchr(char *s, char c) {
        while (*s != 0) {
                if (*s == c) {
                        return s;
                } else {
                        s++;
                }
        }
        return NULL;
}

char *strstr(char *s, char *subs) {
        char *found = NULL;

        while (1) {
                char *ss = subs;
                if (*ss == 0) {
                        return found;
                } else if (*s == 0) {
                        return NULL;
                } else if (*s == *ss) {
                        s += 1;
                        ss += 1;
                } else {
                        s += 1;
                        ss = subs;
                        found = s;
                }
        }
}

char *strcat(char *restrict dest, const char *restrict src) {
        char *end = dest + strlen(dest);
        size_t len = strlen(src);
        size_t i;

        for (i = 0; i < len; i++) {
                end[i] = src[i];
        }

        end[i] = '\0';

        return dest;
}

char *strncat(char *restrict dest, const char *restrict src, size_t max) {
        char *end = dest + strlen(dest);
        size_t len = strlen(src);
        size_t i;

        if (max > len) {
                len = max;
        }

        for (i = 0; i < len; i++) {
                end[i] = src[i];
        }

        end[i] = '\0';

        return dest;
}

void *memchr(void *mem_, char v, size_t count) {
        char *mem = mem_;
        for (int i = 0; i < count; i++, mem++) {
                if (*mem == v) {
                        return mem;
                }
        }
        return NULL;
}

int memcmp(const void *a_, const void *b_, size_t count) {
        const char *a = a_;
        const char *b = b_;

        for (size_t i = 0; i < count; i++) {
                if (a[i] != b[i]) {
                        return a[i] - b[i];
                }
        }
        return 0;
}

void *memset(void *dest_, char value, size_t count) {
        char *dest = dest_;
        for (size_t i = 0; i < count; i++) {
                dest[i] = value;
        }
        return dest;
}

void *wmemset(void *dest_, uint16_t value, size_t count) {
        uint16_t *dest = dest_;
        for (size_t i = 0; i < count; i++) {
                dest[i] = value;
        }
        return dest;
}

void *lmemset(void *dest_, uint32_t value, size_t count) {
        uint32_t *dest = dest_;
        for (size_t i = 0; i < count; i++) {
                dest[i] = value;
        }
        return dest;
}

void *qmemset(void *dest_, uint64_t value, size_t count) {
        uint64_t *dest = dest_;
        for (size_t i = 0; i < count; i++) {
                dest[i] = value;
        }
        return dest;
}

void *memcpy(void *restrict dest_, const void *restrict src_, size_t count) {
        char *dest = dest_;
        const char *src = src_;

        if ((dest < src && dest + count > src) ||
            (dest > src && dest + count < src)) {

                panic_bt("overlapping call to memcpy is invalid");
        }

        for (size_t i = 0; i < count; i++) {
                dest[i] = src[i];
        }

        return dest;
}

void *memmove(void *dest_, const void *src_, size_t count) {
        char *dest = dest_;
        const char *src = src_;

        if (dest < src && dest + count > src) {
                // overlap, dest is lower.
                // no action
        }
        if (dest > src && dest + count < src) {
                // overlap, src is lower.
                // move in reverse
                for (long i = count - 1; i >= 0; i--) {
                        dest[i] = src[i];
                }
                return dest;
        }

        for (size_t i = 0; i < count; i++) {
                dest[i] = src[i];
        }

        return dest;
}

int chr_in(char chr, char *options) {
        for (size_t i=0; options[i]; i++) {
                if (chr == options[i])
                        return 1;
        }

        return 0;
}

/*
 * Reads from source into tok until one of the charcters in delims is
 * encountered.  Leaves a pointer to the rest of the string in rest.
 */
const char *str_until(const char *source, char *tok, char *delims) {
        size_t i;
        const char *rest = NULL;

        for (i=0; source[i]; i++) {
                if (chr_in(source[i], delims))
                        break;
                tok[i] = source[i];
        }

        tok[i] = 0;

        if (source[i])
                rest = &source[i+1];

        return rest;
}

