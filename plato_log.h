#ifndef PLATO_LOG_H
#define PLATO_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>

void pl_log(const char *str, ...);
void pl_logtime(const char *str, ...);
void pl_logbits(void *data, size_t data_sz);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_LOG_IMPLEMENTATION)

void pl_log(const char *str, ...) {
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
}

void pl_logtime(const char *str, ...) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    printf("[%02d:%02d:%02d] ", local->tm_hour, local->tm_min, local->tm_sec);
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
}

void pl_logbits(void *data, size_t data_sz) {
    unsigned char *byte_ptr = (unsigned char*)data;
    for(size_t i = 0; i < data_sz; i++) {
        unsigned char byte = byte_ptr[i];
        for(int j = CHAR_BIT - 1; j >= 0; j--) {
            int bit = (byte >> j) & 1;
            printf("%d", bit);
        }
    }
    printf("\n");
}

#endif // PLATO_LOG_IMPLEMENTATION
#endif // PLATO_LOG_H