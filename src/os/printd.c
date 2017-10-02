
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "board.h"
#include "os/printd.h"
#include "uart.h"
#include "cdc_uart.h"


printd_handler handler = uart_tx;

void printd_set_handler(printd_handler h)
{
	handler = h;
}


static void uitoa(unsigned int value, char* string, unsigned char radix)
{
	signed char index = 0, i = 0;

	/* generate the number in reverse order */
	do {
		string[index] = '0' + (value % radix);
		if (string[index] > '9')
			string[index] += 'a' - '9' - 1;
		value /= radix;
		++index;
	} while (value != 0);

	/* null terminate the string */
	string[index--] = '\0';

	/* reverse the order of digits */
	while (index > i) {
		char tmp = string[i];
		string[i] = string[index];
		string[index] = tmp;
		++i;
		--index;
	}
}

static void itoa(int value, char* string, unsigned char radix)
{
	if (value < 0 && radix == 10) {
		*string++ = '-';
		value = -value;
	}
	uitoa(value, string, radix);
}

void vfprintd(void (*tx)(uint8_t c), const char *fmt, va_list va)
{
	const char *p = fmt;
	char buf[16];
	uint8_t len;
	uint8_t i;
	char pad;
	uint8_t use_long;

	while(*p) {
		if(*p == '%') {
			len = 0;
			use_long = 0;
			pad = ' ';
			p++;
			if(*p == '0') pad = *p++;
			if(isdigit((uint8_t)*p)) len = atoi(p++);

			if(*p == 'l') {
				use_long = 1;
				p++;
			}

			if(*p == 'd' || *p == 'x') {
				if(use_long) {
					long val = va_arg(va, long);
					itoa(val, buf, *p == 'd' ? 10 : 16);
				} else {
					int val = va_arg(va, int);
					itoa(val, buf, *p == 'd' ? 10 : 16);
				}
				for(i=strlen(buf); i<len; i++) tx(pad);
				char *q = buf;
				while(*q) tx(*q++);
			} 
			if(*p == 'u' || *p == 'X') {
				if(use_long) {
					long val = va_arg(va, long);
					uitoa(val, buf, *p == 'd' ? 10 : 16);
				} else {
					int val = va_arg(va, int);
					uitoa(val, buf, *p == 'd' ? 10 : 16);
				}
				for(i=strlen(buf); i<len; i++) tx(pad);
				char *q = buf;
				while(*q) tx(*q++);
			} 
			if(*p == 's') {
				char *q = va_arg(va, char *);
				while(*q) {
					tx(*q++);
				}
			}
			if(*p == 'c') {
				int v = va_arg(va, int);
				tx(v);
			}
			if(*p == '%') {
				tx('%');
			}
		} else if(*p == '\n') {
			tx('\r');
			tx('\n');
		} else {
			tx(*p);
		}
		p++;
	}
}



static void vprintd(const char *fmt, va_list va)
{
	vfprintd(handler, fmt, va);
}


void printd(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vprintd(fmt, va);
	va_end(va);
}


void fprintd(void (*tx)(uint8_t c), const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintd(tx, fmt, va);
	va_end(va);
}


void fhexdump(void (*tx)(uint8_t c), void *addr, size_t len, off_t offset)
{
        int i,j;
	uint8_t *data = addr + offset;

        if(len == 0) return;

        for(i=0; i<len; i+=16) {
                fprintd(tx, "| %08X  ", i + offset);
                for(j=i; (j<i+16);  j++) {
                        if(j<len) {
                                fprintd(tx, "%02X ", (unsigned char)*(data+j));
                        } else {
                                fprintd(tx, "   ");
                        }
			if(j == i+7) fprintd(tx, " ");
                }
                fprintd(tx, "  ");
                for(j=i; (j<i+16) && (j<len);  j++) {
                        if((*(data+j) >= 32) && (*(data+j)<=127)) {
                                fprintd(tx, "%c", *(data+j));
                        } else {
                                fprintd(tx, ".");
                        }
                }
                fprintd(tx, "\n");
        }
}


/*
 * End
 */

