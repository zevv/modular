
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "ctl.h"
#include "biquad.h"

#define DEL_SIZE 10000
#define STEP 14
#define WIN 200
#define NMIX 50
#define DMAX 800

static float delbuf[DEL_SIZE];

struct delay {
	float *buf;
	float feedback;
	int size;
	int head;
};


float readbuf(int ptr, float *buf, int size)
{
	if(ptr < 0) ptr += size;
	if(ptr >= size) ptr -= size;
	return buf[ptr];
}


float read4(float ptr, float *buf, int size)
{
	int i = ptr;
	float f = ptr - i;
	float a = readbuf(i-1, buf, size);
	float b = readbuf(i+0, buf, size);
	float c = readbuf(i+1, buf, size);
	float d = readbuf(i+1, buf, size);
	float c_b = c - b;
	return b + f * ( c_b - 0.16667 * (1.-f) * ( (d - a - 3*c_b) * f + (d + 2*a - 3*b)));
}


void delay_init(struct delay *del, float *buf, int size)
{
	del->buf = buf;
	del->feedback = 0;
	del->size = size;
	del->head = 0;
	memset(del->buf, 0, size * sizeof(del->buf[0]));
}


void delay_write(struct delay *del, float val)
{
	if(val < -1.0) val = -1.0;
	if(val > +1.0) val = +1.0;
	del->buf[del->head] = val;
	del->head ++;
	if(del->head >= del->size) {
		del->head = 0;
	}
}


float delay_read(struct delay *del, float where)
{
	
	int delta = where * del->size;
	if(delta < 0) delta = 0;
	if(delta > del->size) delta = del->size;

	return read4(del->head - where*del->size, del->buf, del->size);
}


void delay_set_feedback(struct delay *del, float fb)
{
	del->feedback = fb;
}


float prand(void)
{
	static uint32_t r = 22222;
	r = (r * 196314165) + 907633515;
	return r / (float)UINT32_MAX;
}


static struct delay del;
static float factor;
static float wet;

void mod_init(void)
{
	delay_init(&del, delbuf, DEL_SIZE);
	ctl_bind_pot(4, &factor, NULL, POT_SCALE_LIN, 0.4, 2.2);
	ctl_bind_pot(5, &wet, NULL, POT_SCALE_LIN, 0, 1);

}


int find(int pf, int pt1, int pt2)
{
	float cmax = 0;
	float ptmax = pt1;

	int pt;
	for(pt=pt1; pt<pt2-WIN; pt+=STEP) {
		float c = 0;
		int i;
		for(i=0; i<WIN; i+=STEP) {
			c += readbuf(pf+i, del.buf, del.size) * readbuf(pt+i, del.buf, del.size);
		}
		if(c > cmax) {
			cmax = c;
			ptmax = pt;
		}
	}

	return ptmax;
}

void mod_run(float *fin, float *fout)
{
	float vi = fin[0];
	static float pr = 0, prn = 0;
	static int mix = 0;

	delay_write(&del, vi);
	float vo = read4(pr, del.buf, del.size);

	if(mix > 0) {
		float f = (float)mix / NMIX;
		vo = vo * f + read4(prn, del.buf, del.size) * (1-f);
		mix = mix - 1;
		if(mix == 0) {
			pr = prn;
		}
	}

	if(mix == 0) {
		int d = del.head - pr;
		while(d < 0) d += del.size;
		while(d >= del.size) d -= del.size;

		if(factor < 1) {
			if(d > DMAX) {
				mix = NMIX;
				prn = find(pr, pr+DMAX*0.5, pr+DMAX);
			}
		} else {
			if(d < WIN || d > DMAX*2) {
				mix = NMIX;
				prn = find(pr, pr-DMAX, pr-WIN);
			}
		}
	}

	pr += factor;
	if(pr > del.size) pr -= del.size;
	
	prn += factor;
	if(prn > del.size) prn -= del.size;

	fout[0] = vi * (1-wet) + vo * wet;
	fout[1] = vi * (1-wet) + vo * wet;
}


struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};


/*
 * End
 */
