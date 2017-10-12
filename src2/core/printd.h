#ifndef bios_printd_h
#define bios_printd_h

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>

typedef void (*printd_handler)(uint8_t);

void printd_set_handler(printd_handler h);
void vfprintd(void (*tx)(uint8_t c), const char *fmt, va_list va);
void printd(const char *fmt, ...); 
void hexdump(void *addr, size_t len, off_t offset);
void fhexdump(void (*tx)(uint8_t c), void *addr, size_t len, off_t offset);
void fprintd(void (*tx)(uint8_t c), const char *fmt, ...);


//__attribute__((format(printf, 1, 2)));

#endif
