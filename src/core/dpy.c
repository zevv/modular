#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "dpy.h"
#include "img.h"
#include "printd.h"
#include "chip.h"
#include "cmd.h"

#include "font-tiny.c"
#include "font-normal.c"
#include "font-bold.c"
#include "icons.c"

static int i2c_addr = 60;

struct font {
	const struct img *img;
	uint8_t cw;
	uint8_t ch;
	uint8_t c_from;
	uint8_t c_to;
};


int dpy_tx(enum dc dc, void *data, size_t len);

struct font font_list[] = {
	[FONT_TINY] = {
		.img = &font_tiny,
		.cw = 5,
		.ch = 8,
		.c_from = '!',
		.c_to = '~',
	},
	[FONT_NORMAL] = {
		.img = &font_normal,
		.cw = 8,
		.ch = 16,
		.c_from = '!',
		.c_to = '~',
	},
#if NEE
	[FONT_BOLD] = {
		.img = &font_bold,
		.cw = 8,
		.ch = 16,
		.c_from = '!',
		.c_to = '~',
	},
#endif
};

/*
 * Connect display SSD1306:
 *
 * SSD1306 pin		AVR/system
 * GND	 		GND
 * Vin	 		+5
 * 3v3   		NC
 * CS    		PB2
 * RST 	 		PB0
 * DC/SA0		PB1
 * SS    		PB4=NC
 * CLK   		PB7/SCK
 * DATA  		PB5/MOSI
 */


#define FONT_WIDTH 8
#define FONT_HEIGHT 14
#define FONT_DPY_OFFSET_X -1
#define FONT_DPY_OFFSET_Y -15

#define CMD_SETCONTRAST 0x81
#define CMD_DISPLAYALLON_RESUME 0xA4
#define CMD_DISPLAYALLON 0xA5
#define CMD_NORMALDISPLAY 0xA6
#define CMD_INVERTDISPLAY 0xA7
#define CMD_DISPLAYOFF 0xAE
#define CMD_DISPLAYON 0xAF
#define CMD_SETDISPLAYOFFSET 0xD3
#define CMD_SETCOMPINS 0xDA
#define CMD_SETVCOMDETECT 0xDB
#define CMD_SETDISPLAYCLOCKDIV 0xD5
#define CMD_SETPRECHARGE 0xD9
#define CMD_SETMULTIPLEX 0xA8
#define CMD_SETLOWCOLUMN 0x00
#define CMD_SETHIGHCOLUMN 0x10
#define CMD_SETSTARTLINE 0x40
#define CMD_MEMORYMODE 0x20
#define CMD_COLUMNADDR 0x21
#define CMD_PAGEADDR 0x22
#define CMD_COMSCANINC 0xC0
#define CMD_COMSCANDEC 0xC8
#define CMD_SEGREMAP 0xA0
#define CMD_CHARGEPUMP 0x8D
#define CMD_EXTERNALVCC 0x1
#define CMD_SWITCHCAPVCC 0x2
#define CMD_ACTIVATE_SCROLL 0x2F
#define CMD_DEACTIVATE_SCROLL 0x2E
#define CMD_SET_VERTICAL_SCROLL_AREA 0xA3
#define CMD_RIGHT_HORIZONTAL_SCROLL 0x26
#define CMD_LEFT_HORIZONTAL_SCROLL 0x27
#define CMD_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define CMD_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A


static void dpy_reset(void);
static int dpy_cmd(uint8_t c);

static uint8_t fb[LCD_WIDTH * LCD_HEIGHT / 8];

void dpy_init(void)
{
	Chip_I2C_SetClockRate(0, 1000000);

	dpy_reset();

	dpy_cmd(CMD_DISPLAYOFF);
	dpy_cmd(CMD_SETDISPLAYCLOCKDIV); dpy_cmd(0x80); 
	dpy_cmd(CMD_SETMULTIPLEX);       dpy_cmd(LCD_HEIGHT - 1);
	dpy_cmd(CMD_SETDISPLAYOFFSET);   dpy_cmd(0x00);
	dpy_cmd(CMD_SETSTARTLINE | 0x0);
	dpy_cmd(CMD_SETCOMPINS);         dpy_cmd(0x12);

	dpy_cmd(CMD_CHARGEPUMP);         dpy_cmd(0x14);

	dpy_cmd(CMD_MEMORYMODE);         dpy_cmd(0x00);
	dpy_cmd(CMD_SEGREMAP | 0x1);
	dpy_cmd(CMD_COMSCANDEC);

	dpy_cmd(CMD_SETPRECHARGE);       dpy_cmd(0x32);
	dpy_cmd(CMD_SETVCOMDETECT);      dpy_cmd(0x40);
	dpy_cmd(CMD_DISPLAYALLON_RESUME);
	dpy_cmd(CMD_NORMALDISPLAY);
	dpy_cmd(CMD_DISPLAYON);

	dpy_brightness(255);
}


/*
 * Set display brightness from 0 to 255
 */

void dpy_brightness(uint8_t v)
{
	dpy_cmd(CMD_SETCONTRAST);
	dpy_cmd(v);
}


static void dpy_reset(void)
{
}


int dpy_tx(enum dc dc, void *data, size_t len)
{
	uint8_t buf[len+1];
	buf[0] = (dc == DC_CMD) ? 0x00 : 0x40;

	memcpy(buf+1, data, len);
	return Chip_I2C_MasterSend(0, i2c_addr, buf, len+1);
}


static int dpy_cmd(uint8_t c)
{
	return dpy_tx(DC_CMD, &c, 1);
}


static bool dpy_pget(uint8_t x, uint8_t y)
{
	uint16_t o = (y/8) * LCD_WIDTH + x;
	uint8_t b = y & 0x07;
	return !!(fb[o] & (1<<b));
}


#define UTFBUFSIZ 8

void print_utf8(struct cmd_cli *cli, uint32_t x)
{
	size_t n = 1;
	char buf[UTFBUFSIZ+1];

	if (x < 0x80) {
		buf[UTFBUFSIZ - 1] = (char)x;
	} else {
		uint32_t mfb = 0x3f;
		do {
			buf[UTFBUFSIZ - (n++)] = (char)(0x80 | (x & 0x3f));
			x >>= 6;
			mfb >>= 1;
		} while (x > mfb);
		buf[UTFBUFSIZ - n] = (char)((~mfb << 1) | x);
	}
	buf[UTFBUFSIZ] = '\0';
	cmd_printd(cli, "%s", buf + UTFBUFSIZ - n);
}


void dpy_flush(void)
{
	dpy_cmd(CMD_PAGEADDR);
	dpy_cmd(0);
	dpy_cmd(7);

	dpy_cmd(CMD_COLUMNADDR);
	dpy_cmd(0);
	dpy_cmd(127);

	dpy_cmd(CMD_MEMORYMODE);
	dpy_cmd(0x00);

	size_t i;
	for(i=0; i<sizeof(fb)-8; i+=64) {
		dpy_tx(DC_DATA, fb+i, 64);
	}
}


void dpy_print(struct cmd_cli *cli)
{
	int y, x;
	for(y=0; y<LCD_HEIGHT; y+=4) {
		for(x=0; x<LCD_WIDTH; x+=2) {
			uint32_t v = 0x2800;
			if(dpy_pget(x+0, y+0)) v |= 0x01;
			if(dpy_pget(x+1, y+0)) v |= 0x08;
			if(dpy_pget(x+0, y+1)) v |= 0x02;
			if(dpy_pget(x+1, y+1)) v |= 0x10;
			if(dpy_pget(x+0, y+2)) v |= 0x04;
			if(dpy_pget(x+1, y+2)) v |= 0x20;
			if(dpy_pget(x+0, y+3)) v |= 0x40;
			if(dpy_pget(x+1, y+3)) v |= 0x80;
			print_utf8(cli, v);
		}
		cmd_printd(cli, "\n");
	}
}


void dpy_clear(void)
{
	memset(fb, 0x00, sizeof fb);
}


void dpy_pset(uint8_t x, uint8_t y, uint8_t v)
{
	if(x < 128 && y < 64) {

		uint16_t o = (y/8) * LCD_WIDTH + x;
		uint8_t b = y & 0x07;

		if(v) {
			fb[o] |= (1<<b);
		} else {
			fb[o] &= ~(1<<b);
		}
	}
}

#define swap(a,b) { int _tmpswap = a; a = b; b = _tmpswap; } 

void dpy_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t v)
{
	int i;
	int steep = 0;
	int sx, sy;	
	int dx, dy;
	int e;

	/*
	 * Draw the line using the bresenham algorithm. 
	 */

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	sx = ((x2 - x1) > 0) ? 1 : -1;
	sy = ((y2 - y1) > 0) ? 1 : -1;

	if (dy > dx) {
		steep = 1;
		swap(x1, y1);
		swap(dx, dy);
		swap(sx, sy);
	}

	e = (dy << 1) - dx;
	for (i = 0; i <= dx; i++) {
		if (steep) {
			dpy_pset(y1, x1, v);
		} else {
			dpy_pset(x1, y1, v);
		}
		while (e >= 0) {
			y1 += sy;
			e -= (dx << 1);
		}
		x1 += sx;
		e += (dy << 1);
	}
}


void dpy_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t v)
{

	dpy_line( x1, y1, x2, y1, v);   //  ---
	dpy_line( x1, y1, x1, y2, v);   //  |
	dpy_line( x2, y1, x2, y2, v);   //    |
	dpy_line( x1, y2, x2, y2 ,v);   //  ___

}


void dpy_circle(int8_t x0, int8_t y0, int8_t radius, uint8_t v)
{
	int8_t x = radius, y = 0;
	int8_t radiusError = 1-x;

	while(x >= y)
	{
		dpy_pset( x + x0,  y + y0, v);
		dpy_pset( y + x0,  x + y0, v);
		dpy_pset(-x + x0,  y + y0, v);
		dpy_pset(-y + x0,  x + y0, v);
		dpy_pset(-x + x0, -y + y0, v);
		dpy_pset(-y + x0, -x + y0, v);
		dpy_pset( x + x0, -y + y0, v);
		dpy_pset( y + x0, -x + y0, v);

		y++;

		if (radiusError<0) {
			radiusError += 2 * y + 1;
		} else {
			x--;
			radiusError += 2 * (y - x + 1);
		}
	}
}


uint8_t rev(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}


/*
 * Clear area. assume mulitple of 8 
 */

void dpy_clear_rect(uint8_t dx, uint8_t dy, uint8_t w, uint8_t h)
{
	uint8_t x, y;
	for(y=0; y<h; y+=8) {
		uint8_t *pd = fb + LCD_WIDTH * (dy + y) / 8 + dx;
		for(x=0; x<w; x++) {
			*pd++ = 0;
		}
	}
}


/*
 * Blit img to framebuffer. dy and h should be multiple of 8.
 */

void dpy_blit(const struct img *src, uint16_t sx, uint8_t sy, uint8_t dx, uint8_t dy, uint8_t w, uint8_t h)
{
	uint8_t x, y;

	uint16_t sw = src->w;
	uint16_t sh = src->h;

	if(w > sw) w = sw;
	if(h > sh) h = sh;

	for(y=0; y<h; y+=8) {

		const uint8_t *ps = src->data + sw * (sy + y) / 8 + sx;
		uint8_t *pd = fb + LCD_WIDTH * (dy + y) / 8 + dx;

		for(x=0; x<w; x++) {
			*pd++ = *ps++;
		}
	}
}


#if NEE
void dpy_icon(enum icon_id id, uint8_t x, uint8_t y)
{
	dpy_blit(&icons, (id-1)*16, 0, x, y, 16, 16);
}
#endif

int dpy_text(enum font_id font_id, int xpos, int ypos, char *s)
{
	struct font *font = &font_list[font_id];

	while(*s) {
		if(*s < font->c_from || *s > font->c_to) {
			dpy_clear_rect(xpos, ypos, font->cw, font->ch);
		} else {
			uint16_t sx = (*s - font->c_from) * font->cw;
			dpy_blit(font->img, sx, 0, xpos, ypos, font->cw, font->ch);
		}
		xpos += font->cw;
		s++;
	}

	return font->ch;
}


void dpy_printf(enum font_id font_id, int xpos, int ypos, char *fmt, ...)
{
	char buf[26];
	va_list va;
	va_start(va, fmt);
	vsnprintd(buf, sizeof buf, fmt, va);
	va_end(va);
	dpy_text(font_id, xpos, ypos, buf);
}


void dpy_invert(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	uint8_t y, x;
	for(y=y1; y<y2; y+=8) {
		for(x=x1; x<x2; x++) {
			int o = (y/8) * LCD_WIDTH + x;
			fb[o] ^= 0xff;
		}
	}
}

static void corner(uint8_t x, uint8_t y, uint8_t *mask)
{
	uint8_t i;
	uint8_t *p = fb + (y/8) * LCD_WIDTH + x;
	for(i=0; i<4; i++) *p++ ^= *mask++;
}


void dpy_invert2(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	dpy_invert(x1, y1, x2, y2);

	uint8_t mask[4][4] = {
		{ 0x0f, 0x03, 0x01, 0x01 },
		{ 0x01, 0x01, 0x03, 0x0f },
		{ 0xf0, 0xc0, 0x80, 0x80 },
		{ 0x80, 0x80, 0xc0, 0xf0 },
	};

	corner(x1,   y1,   mask[0]);
	corner(x2-4, y1,   mask[1]);
	corner(x1,   y2-8, mask[2]);
	corner(x2-4, y2-8, mask[3]);
}


static int on_cmd_dpy(struct cmd_cli *cli, uint8_t argc, char **argv)
{
	dpy_init();
	dpy_clear();
	dpy_line(0, 0, 30, 30, 1);
	dpy_text(FONT_BOLD, 10, 10, "HALLO");
	dpy_flush();
	return 1;
}


CMD_REGISTER(dpy, on_cmd_dpy, "");


/*
 * End
 */
