
#include <string.h>
#include <stdlib.h>

#include "module.h"
#include "osc.h"
#include "biquad.h"
#include "ctl.h"

#define DEL_SIZE 55000
#define MAX_TAPS 8

static struct osc osc, lfo[MAX_TAPS];

static int q;
static float c1;
static float c2;
static float c3;



static __fp16 delbuf[DEL_SIZE];
static struct biquad hp;

struct delay {
	__fp16 *buf;
	float feedback;
	int size;
	int head;
};


float readbuf(int ptr, __fp16 *buf, int size)
{
	if(ptr < 0) ptr += size;
	if(ptr >= size) ptr -= size;
	return buf[ptr];
}

float read4(float ptr, __fp16 *buf, int size)
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


void delay_init(struct delay *del, __fp16 *buf, int size)
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
static float where;
static float feedback;
static float lfofreq;


static void on_set(void)
{
	delay_set_feedback(&del, feedback);
	int i;
	for(i=0; i<MAX_TAPS; i++) {
		osc_init(&lfo[i], SRATE);
		osc_set_freq(&lfo[i], lfofreq * (1 + i * 0.1));
	}
}


void mod_init(void)
{
	q = 15;
	c1 = (1 << q) - 1;
	c2 = ((int)(c1 / 3)) + 1;
	c3 = 1.f / c1;
	
	
	biquad_init(&hp, SRATE);
	biquad_config(&hp, BIQUAD_TYPE_HP, 30, 0.707);

	osc_init(&osc, SRATE);
	osc_set_type(&osc, OSC_TYPE_SIN);
	delay_init(&del, delbuf, DEL_SIZE);

	int i;
	for(i=0; i<MAX_TAPS; i++) {
		osc_init(&lfo[i], SRATE);
	}

	ctl_bind_pot(4, &where, on_set, POT_SCALE_LOG, 0, 1);
	ctl_bind_pot(5, &lfofreq, on_set, POT_SCALE_LOG, 0.5, 10);
	ctl_bind_pot(6, &feedback, on_set, POT_SCALE_LIN, 0, 1);
}


void mod_run(float *fin, float *fout)
{
	static int n = 0;
	float v = osc_gen(&osc);
	if(n++ >= SRATE) {
		osc_set_freq(&osc, prand() * 500 + 10);
		n = 0;
	}

	float vl = 0, vr = 0;

	vl += delay_read(&del, where * 0.10 + osc_gen(&lfo[0]) * 0.001) * 0.40;
	vl += delay_read(&del, where * 0.30 + osc_gen(&lfo[1]) * 0.001) * 0.10;
	vl += delay_read(&del, where * 0.50 + osc_gen(&lfo[2]) * 0.001) * 0.05;
	vl += delay_read(&del, where * 0.70 + osc_gen(&lfo[3]) * 0.001) * 0.02;
	
	vr += delay_read(&del, where * 0.15 + osc_gen(&lfo[4]) * 0.001) * 0.40;
	vr += delay_read(&del, where * 0.35 + osc_gen(&lfo[5]) * 0.001) * 0.20;
	vr += delay_read(&del, where * 0.55 + osc_gen(&lfo[6]) * 0.001) * 0.10;
	vr += delay_read(&del, where * 0.75 + osc_gen(&lfo[7]) * 0.001) * 0.05;

	fout[0] = vl * 0.5;
	fout[1] = vr * 0.5;

	(void)v;
	delay_write(&del, biquad_run(&hp, fin[0] + (vr + vl) * del.feedback * -1.75));
}


struct module mod = {
	.init = mod_init,
	.run_float = mod_run,
};


/*
 * End
 */
