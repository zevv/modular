#ifndef bios_printd_h
#define bios_printd_h

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

typedef void (*printd_handler)(uint8_t c);

void vfprintd(printd_handler tx, const char *fmt, va_list va);
void fprintd(printd_handler tx, const char *fmt, ...);

void vsnprintd(char *str, size_t size, const char *fmt, va_list va);

void snprintd(char *str, size_t size, const char *fmt, ...);

void printd(const char *fmt, ...);

void hexdump(const void *addr, size_t len, uintptr_t offset);
void fhexdump(printd_handler tx, const void *addr, size_t len, uintptr_t offset);


//__attribute__((format(printf, 1, 2)));

void printd_set_handler(printd_handler h);

#endif
