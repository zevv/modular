

#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include "uart.h"
#include "printd.h"

static printd_handler handler = NULL;

unsigned xvformat(printd_handler tx, const char *fmt, va_list args);


struct param_s {
	union {
		unsigned long lvalue;
		float dvalue;
	} values;

	char *out;
	float dbl;
	unsigned long fPart;
	int length;
	int prec;
	int width;
	unsigned count;
	unsigned flags;

#define FLAG_TYPE_INT 0x0000      /* Argument is integer     */
#define FLAG_TYPE_LONG 0x0001     /* Argument is long      */
#define FLAG_TYPE_SIZEOF 0x0002   /* Argument is size_t     */
#define FLAG_TYPE_LONGLONG 0x0003 /* Argument is long long    */
#define FLAG_TYPE_MASK 0x0003     /* Mask for field type     */
#define FLAG_PREC 0x0004          /* Precision set      */
#define FLAG_LEFT 0x0008          /* Left alignment      */
#define FLAG_BLANK 0x0010         /* Blank before positive integer number */
#define FLAG_PREFIX 0x0020        /* Prefix required      */
#define FLAG_PLUS 0x0040          /* Force a + before positive number  */
#define FLAG_UPPER 0x0080         /* Output in upper case letter   */
#define FLAG_DECIMAL 0x0100       /* Decimal field      */
#define FLAG_INTEGER 0x0200       /* Integer field      */
#define FLAG_MINUS 0x0400         /* Field is negative     */
#define FLAG_VALUE 0x0800         /* Value set       */
#define FLAG_BUFFER 0x1000        /* Buffer set       */

	int prefixlen;
	char prefix[2];
	char buffer[sizeof(long) * 8 + 1];
	unsigned char radix;
	char pad;
	char state;
};


enum State {
	ST_NORMAL = 0,
	ST_PERCENT = 1,
	ST_FLAG = 2,
	ST_WIDTH = 3,
	ST_DOT = 4,
	ST_PRECIS = 5,
	ST_SIZE = 6,
	ST_TYPE = 7
};


enum CharClass {
	CH_OTHER = 0,
	CH_PERCENT = 1,
	CH_DOT = 2,
	CH_STAR = 3,
	CH_ZERO = 4,
	CH_DIGIT = 5,
	CH_FLAG = 6,
	CH_SIZE = 7,
	CH_TYPE = 8
};


static const char ms_null[] = "(null)";
static const char ms_digits[] = "0123456789abcdef";

/*
 * This table contains the next state for all char and it will be
 * generate using xformattable.c
 */

static const unsigned char formatStates[] = {
    0x06, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x03, 0x06,
    0x00, 0x06, 0x02, 0x10, 0x04, 0x45, 0x45, 0x45, 0x45, 0x05, 0x05, 0x05,
    0x05, 0x35, 0x30, 0x00, 0x50, 0x60, 0x00, 0x00, 0x00, 0x20, 0x28, 0x38,
    0x50, 0x50, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x50, 0x50, 0x00, 0x00,
    0x08, 0x20, 0x20, 0x28, 0x20, 0x20, 0x20, 0x00, 0x08, 0x60, 0x60, 0x60,
    0x60, 0x60, 0x60, 0x00, 0x00, 0x70, 0x78, 0x78, 0x78, 0x70, 0x78, 0x00,
    0x07, 0x08, 0x00, 0x00, 0x07, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x08,
    0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x07 };


/**
 * Convert an unsigned value in one string
 *
 * All parameter are in the passed structure
 *
 * @param prec		- Minimum precision
 * @param lvalue	- Unsigned value
 * @param radix		- Radix (Supported values 2/8/10/16)
 *
 * @param out		- Buffer with the converted value.
 */

static void ulong2a(struct param_s *param)
{
	unsigned char digit;

	while(param->prec-- > 0 || param->values.lvalue) {
		switch(param->radix) {
			case 2:
				digit = param->values.lvalue & 0x01;
				param->values.lvalue >>= 1;
				break;

			case 8:
				digit = param->values.lvalue & 0x07;
				param->values.lvalue >>= 3;
				break;

			case 16:
				digit = param->values.lvalue & 0x0F;
				param->values.lvalue >>= 4;
				break;

			default:
			case 10:
				digit = param->values.lvalue % 10;
				param->values.lvalue /= 10;
				break;
		}


		*param->out-- = ms_digits[digit];
		param->length++;
	}
}


static unsigned outBuffer(printd_handler tx, const char *buffer, int len, unsigned toupper)
{
	unsigned count = 0;
	int i;
	char c;

	for(i = 0; i < len; i++) {
		c = buffer[i];

		if(toupper && (c >= 'a' && c <= 'z')) {
			c -= 'a' - 'A';
		}

		tx(c);
		count++;
	}

	return count;
}


static unsigned outChars(printd_handler tx, char ch, int len)
{
	unsigned count = 0;

	while(len-- > 0) {
		tx(ch);
		count++;
	}

	return count;
}


/**
 * Printf like format function.
 *
 * General format :
 *
 * %[width][.precision][flags]type
 *
 * - width Is the minimum size of the field.
 *
 * - precision Is the maximum size of the field.
 *
 * Supported flags :
 *
 * - l	With integer number the argument will be of type long.
 * - z	Compatible with C99 the argument is size_t (aka sizeof(void *))
 * - +	A + sign prefix positive number.
 * - #	A prefix will be printed (o for octal,0x for hex,0b for binary)
 * - 0	Value will be padded with zero (default is spacwe)
 * - -	Left justify as default filed have rigth justification.
 *
 * Supported type :
 *
 * - s	Null terminated string_t of char.
 * - i	Integer number.
 * - d	Integer number.
 * - u	Unsigned number.
 * - x	Unsigned number in hex.
 * - X	Unsigned number in hex upper case.
 * - b	Binary number
 * - o	Octal number
 * - p	Pointer will be emitted with the prefix ->
 * - P	Pointer in upper case letter.
 * - f	Floating point number.
 *
 * @param outchar - Pointer to the function to output one char.
 * @param ptr     - Argument for the output function.
 * @param fmt     - Format options for the list of parameters.
 * @param args    - List parameters.
 *
 * @return The number of char emitted.
 */

unsigned xvformat(printd_handler tx, const char *fmt, va_list args)
{
	struct param_s param;
	int i;
	char c;

	param.count = 0;
	param.state = ST_NORMAL;

	while(*fmt) {
		c = *fmt++;

		if(c < ' ' || c > 'z')
			i = (int)CH_OTHER;
		else
			i = formatStates[c - ' '] & 0x0F;

		param.state = formatStates[(i << 3) + param.state] >> 4;


		switch(param.state) {
		default:
		case ST_NORMAL:
			tx(c);
			param.count++;
			break;

		case ST_PERCENT:
			param.length = param.prefixlen = param.width = param.prec = 0;
			param.flags = 0;
			param.pad = ' ';
			break;

		case ST_WIDTH:
			if(c == '*') {
				param.width = (int)va_arg(args, int);
			} else {
				param.width = param.width * 10 + (c - '0');
			}
			break;

		case ST_DOT:
			break;

		case ST_PRECIS:
			param.flags |= FLAG_PREC;
			if(c == '*') {
				param.prec = (int)va_arg(args, int);
			} else {
				param.prec = param.prec * 10 + (c - '0');
			}
			break;

		case ST_SIZE:
			switch(c) {
			default:
				break;
			case 'z':
				param.flags &= ~FLAG_TYPE_MASK;
				param.flags |= FLAG_TYPE_SIZEOF;
				break;

			case 'l':
				param.flags &= ~FLAG_TYPE_MASK;
				param.flags |= FLAG_TYPE_LONG;
				break;
			}
			break;

		case ST_FLAG:
			switch(c) {
			default:
				break;
			case '-':
				param.flags |= FLAG_LEFT;
				break;
			case '0':
				param.pad = '0';
				break;
			case ' ':
				param.flags |= FLAG_BLANK;
				break;
			case '#':
				param.flags |= FLAG_PREFIX;
				break;
			case '+':
				param.flags |= FLAG_PLUS;
				break;
			}
			break;

		case ST_TYPE:
			switch(c) {
			default:
				param.length = 0;
				break;

			/*
			 * Pointer upper case
			 */
			case 'P':
				param.flags |= FLAG_UPPER;
			/* no break */
			/*lint -fallthrough */

			/*
			 * Pointer
			 */
			case 'p':
				param.flags |= FLAG_INTEGER;
				param.flags &= ~FLAG_TYPE_MASK;
				param.flags |= FLAG_TYPE_SIZEOF;
				param.radix = 16;
				param.prec = sizeof(void *) * 2;
				param.prefixlen = 0;
				break;

			/*
			 * Binary number
			 */
			case 'b':
				param.flags |= FLAG_INTEGER;
				param.radix = 2;
				if(param.flags & FLAG_PREFIX) {
					param.prefix[0] = '0';
					param.prefix[1] = 'b';
					param.prefixlen = 2;
				}
				break;

			/*
			 * Octal number
			 */
			case 'o':
				param.flags |= FLAG_INTEGER;
				param.radix = 8;
				if(param.flags & FLAG_PREFIX) {
					param.prefix[0] = '0';
					param.prefixlen = 1;
				}
				break;

			/*
			 * Hex number upper case letter.
			 */
			case 'X':
				/* no break */
				param.flags |= FLAG_UPPER;

			/* no break */

			/* lint -fallthrough */

			/*
			 * Hex number lower case
			 */
			case 'x':
				param.flags |= FLAG_INTEGER;
				param.radix = 16;
				if(param.flags & FLAG_PREFIX) {
					param.prefix[0] = '0';
					param.prefix[1] = 'x';
					param.prefixlen = 2;
				}
				break;

			/*
			 * Integer number radix 10
			 */
			case 'd':
			case 'i':
				param.flags |=
				    FLAG_DECIMAL | FLAG_INTEGER;
				param.radix = 10;
				break;

			/*
			 * Unsigned number
			 */
			case 'u':
				param.flags |= FLAG_INTEGER;
				param.radix = 10;
				break;


			/*
			 * Normal string
			 */
			case 's':
				param.out =
				    va_arg(args, char *);
				if(param.out == 0)
					param.out =
					    (char *)ms_null;
				param.length =
				    (int)strlen(param.out);
				break;

			/*
			 * Char
			 */
			case 'c':
				param.out = param.buffer;
				param.buffer[0] =
				    (char)va_arg(args, int);
				param.length = 1;
				break;

			/**
			 * Floating point number
			 */
			case 'f':
				if(!(param.flags & FLAG_PREC)) {
					param.prec = 6;
				}

				param.dbl = va_arg(args, double);
				param.values.dvalue = 0.51;
				for(i=0; i<param.prec; i++) {
					param.values.dvalue /= 10.0;
				}

				if(param.dbl < 0) {
					param.flags |= FLAG_MINUS;
					param.dbl -= param.values.dvalue;
					param.fPart = (long)param.dbl;
					param.dbl -= (long)param.fPart;
					param.dbl = -param.dbl;
				} else {
					param.dbl += param.values.dvalue;
					param.fPart = (long)param.dbl;
					param.dbl -= param.fPart;
				}

				for(i = 0; i < param.prec; i++) {
					param.dbl *= 10.0;
				}

				param.values.lvalue = (unsigned long)param.dbl;

				param.out = param.buffer + sizeof(param.buffer) - 1;
				param.radix = 10;
				if(param.prec) {
					ulong2a(&param);
					*param.out-- = '.';
					param.length++;
				}
				param.flags |= FLAG_INTEGER | FLAG_BUFFER | FLAG_DECIMAL | FLAG_VALUE | FLAG_TYPE_LONG;

				param.prec = 0;
				param.values.lvalue = (unsigned long)param.fPart;
				break;

			}

			/*
			 * Process integer number
			 */

			if(param.flags & FLAG_INTEGER) {
				if(param.prec == 0) param.prec = 1;

				if(!(param.flags & FLAG_VALUE)) {
					if(param.flags & FLAG_DECIMAL) {
						switch(param.flags & FLAG_TYPE_MASK) {
							case FLAG_TYPE_SIZEOF:
								param.values.lvalue = (unsigned long) va_arg( args, void *);
								break;
							case FLAG_TYPE_LONG:
								param.values.lvalue = (long)va_arg(args, long);
								break;
							case FLAG_TYPE_INT:
								param.values.lvalue = (long)va_arg( args, int);
								break;
						}

					} else {
						switch(param.flags &
						       FLAG_TYPE_MASK) {
							case FLAG_TYPE_SIZEOF:
								param.values.lvalue = (unsigned long) va_arg( args, void *);
								break;
							case FLAG_TYPE_LONG:
								param.values.lvalue = (unsigned long) va_arg( args, unsigned long);
								break;
							case FLAG_TYPE_INT:
								param.values.lvalue = (unsigned long) va_arg( args, unsigned int);
								break;
						}
					}
				}

				if((param.flags & FLAG_PREFIX) &&
				   param.values.lvalue == 0) {
					param.prefixlen = 0;
				}


				/*
				 * Manage signed integer
				 */
				if(param.flags & FLAG_DECIMAL) {
					if((long)param.values.lvalue <
					   0) {
						param.values.lvalue = ~param.values .lvalue + 1;
						param.flags |= FLAG_MINUS;
					}
					if(!(param.flags &
					     FLAG_MINUS) &&
					   (param.flags & FLAG_BLANK)) {
						param.prefix[0] = ' ';
						param.prefixlen = 1;
					}
				}

				if((param.flags & FLAG_BUFFER) == 0) {
					param.out = param.buffer + sizeof(param.buffer) - 1;
				}

				ulong2a(&param);
				param.out++;

				/*
				 * Check if a sign is required
				 */
				if(param.flags & (FLAG_MINUS | FLAG_PLUS)) {
					c = param.flags & FLAG_MINUS ? '-' : '+';

					if(param.pad == '0') {
						param.prefixlen = 1;
						param.prefix[0] = c;
					} else {
						*--param.out = c;
						param.length++;
					}
				}


			} else {
				if(param.width &&
				   param.length > param.width) {
					param.length = param.width;
				}
			}

			/*
			 * Now width contain the size of the pad
			 */

			param.width -= (param.length + param.prefixlen);

			param.count += outBuffer(tx,
			    param.prefix, param.prefixlen,
			    param.flags & FLAG_UPPER);
			if(!(param.flags & FLAG_LEFT))
				param.count +=
				    outChars(tx, param.pad,
					     param.width);
			param.count += outBuffer(tx,
			    param.out, param.length,
			    param.flags & FLAG_UPPER);
			if(param.flags & FLAG_LEFT)
				param.count +=
				    outChars(tx, param.pad,
					     param.width);
		}
	}

#if XCFG_FORMAT_VA_COPY
	va_end(args);
#endif

	return param.count;
}
	

void vfprintd(printd_handler tx, const char *fmt, va_list va)
{
	(void)xvformat(tx, fmt, va);
}


struct string_printd_handler_data {
	char *p;
	size_t size;
};



void snprintd(char *str, size_t size, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vsnprintd(str, size, fmt, va);
	va_end(va);
}


static void uart_printd_handler(uint8_t c)
{ 
	uart_tx(c); 
}


void printd(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintd(handler, fmt, va);
	va_end(va);
}


void fprintd(printd_handler tx, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintd(tx, fmt, va);
	va_end(va);
}


void fhexdump(printd_handler tx, const void *addr, size_t len, uintptr_t offset)
{
	const uint8_t *data = addr;

	if(len == 0) return;

	size_t i, j;
	for(i = 0; i < len; i += 16) {
		fprintd(tx, "%08lX  ", (long)(i + offset));
		for(j = i; (j < i + 16); j++) {
			if(j < len) {
				fprintd(tx, "%02X ",
					(unsigned char)*(data + j));
			} else {
				fprintd(tx, "   ");
			}
			if(j == i + 7) fprintd(tx, " ");
		}
		fprintd(tx, "  |");
		for(j = i; (j < i + 16) && (j < len); j++) {
			if((*(data + j) >= 32) && (*(data + j) <= 127)) {
				fprintd(tx, "%c", *(data + j));
			} else {
				fprintd(tx, ".");
			}
		}
		fprintd(tx, "|\n");
	}
}


void hexdump(const void *addr, size_t len, uintptr_t offset)
{
	fhexdump(uart_printd_handler, addr, len, offset);
}


void printd_set_handler(printd_handler h)
{
	handler = h;
}


/*
 * End
 */
