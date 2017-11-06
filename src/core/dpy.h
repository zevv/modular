#ifndef dpy_h 
#define dpy_h

#include "img.h"

enum font_id {
	FONT_NORMAL,
	FONT_BOLD,
	FONT_TINY,
};

enum icon_id {
	ICON_ALARM = 1,
	ICON_CLOCK,
	ICON_WHEEL,
	ICON_BRIGHTNESS,
	ICON_INFO,
	ICON_TACHO,
	ICON_CHARGING,
	ICON_BATTERY,
	ICON_S1,
	ICON_S2,
	ICON_S3,
	ICON_S4,
	ICON_S5
};

enum dc {
	DC_CMD,
	DC_DATA,
};

#define LCD_WIDTH 128
#define LCD_HEIGHT 64

void dpy_open(void);
void dpy_init(void);

void dpy_clear(void);
void dpy_clear_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void dpy_flush(void);
void dpy_brightness(uint8_t b);
void dpy_pset(uint8_t x, uint8_t y, uint8_t v);
void dpy_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t v);
void dpy_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t v);
void dpy_circle(int8_t x0, int8_t y0, int8_t radius, uint8_t v);
void dpy_blit(const struct img *src, uint16_t sx, uint8_t sy, uint8_t dx, uint8_t dy, uint8_t w, uint8_t h);
void dpy_icon(enum icon_id, uint8_t x, uint8_t y);
int  dpy_text(enum font_id font_id, int xpos, int ypos, char *s);
void dpy_printf(enum font_id font_id, int xpos, int ypos, char *fmt, ...);
void dpy_invert(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void dpy_invert2(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#endif 
