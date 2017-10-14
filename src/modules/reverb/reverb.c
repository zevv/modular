#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "reverb.h"

#define	fixedgain 0.007f
#define scalewet 3.0f
#define scaledamp 1.0f
#define scaleroom 0.28f
#define offsetroom 0.7f
#define initialroom 0.3f
#define initialdamp 0.1f
#define initialwet 1
#define initialdry 0
#define initialwidth 1.0
#define stereospread 23


/*
 * These values assume 44.1KHz sample rate
 * they will probably be OK for 48KHz sample rate
 * but would need scaling for 96KHz (or other) sample rates.
 * The values were obtained by listening tests.
 */

#define combtuningL1 1116
#define combtuningR1 (1116 + stereospread)
#define combtuningL2 1188
#define combtuningR2 (1188 + stereospread)
#define combtuningL3 1277
#define combtuningR3 (1277 + stereospread)
#define combtuningL4 1356
#define combtuningR4 (1356 + stereospread)
#define combtuningL5 1422
#define combtuningR5 (1422 + stereospread)
#define combtuningL6 1491
#define combtuningR6 (1491 + stereospread)
#define combtuningL7 1557
#define combtuningR7 (1557 + stereospread)
#define combtuningL8 1617
#define combtuningR8 (1617 + stereospread)
#define allpasstuningL1 556
#define allpasstuningR1 (556 + stereospread)
#define allpasstuningL2 441
#define allpasstuningR2 (441 + stereospread)
#define allpasstuningL3 341
#define allpasstuningR3 (341 + stereospread)
#define allpasstuningL4 225
#define allpasstuningR4 (225 + stereospread)



/** Flags for revmodel_set() */
typedef enum
{
	REVMODEL_SET_ROOMSIZE       = 1 << 0,
	REVMODEL_SET_DAMPING        = 1 << 1,
	REVMODEL_SET_WIDTH          = 1 << 2,
	REVMODEL_SET_LEVEL          = 1 << 3
} revmodel_set_t;

/** Value for revmodel_set() which sets all reverb parameters. */
#define REVMODEL_SET_ALL      0x0F


revmodel_t* new_revmodel(float sample_rate);

void revmodel_reset(revmodel_t* rev);

void revmodel_set(revmodel_t* rev, int set, float roomsize,
		float damping, float width, float level);



/* Denormalizing offset */

#define DC_OFFSET 1e-8



void allpass_init(allpass* allpass);
void allpass_setfeedback(allpass* allpass, float val);
float allpass_getfeedback(allpass* allpass);

void allpass_setbuffer(allpass* allpass, int size)
{
	allpass->bufidx = 0;
	allpass->bufsize = size;
}


void allpass_init(allpass* allpass)
{
	int i;
	int len = allpass->bufsize;
	float* buf = allpass->buffer;
	for (i = 0; i < len; i++) {
		buf[i] = DC_OFFSET; /* this is not 100 % correct. */
	}
}


void allpass_setfeedback(allpass* allpass, float val)
{
	allpass->feedback = val;
}


float allpass_getfeedback(allpass* allpass)
{
	return allpass->feedback;
}


float allpass_process(allpass* allpass, float input)
{ 
	float output; 
	float bufout; 
	bufout = allpass->buffer[allpass->bufidx];
	output = -input + bufout;
	allpass->buffer[allpass->bufidx] = input + (bufout * allpass->feedback); 
	if (++allpass->bufidx >= allpass->bufsize) { 
		allpass->bufidx = 0; 
	} 
	return output;
} 



void comb_setbuffer(comb* comb, int size);
void comb_release(comb* comb);
void comb_init(comb* comb);
void comb_setdamp(comb* comb, float val);
float comb_getdamp(comb* comb);
void comb_setfeedback(comb* comb, float val);
float comb_getfeedback(comb* comb);


void comb_setbuffer(comb* comb, int size)
{
	comb->filterstore = 0;
	comb->bufidx = 0;
	comb->bufsize = size;
}


void comb_init(comb* comb)
{
	int i;
	float* buf = comb->buffer;
	int len = comb->bufsize;
	for (i = 0; i < len; i++) {
		buf[i] = DC_OFFSET; /* This is not 100 % correct. */
	}
}


void comb_setdamp(comb* comb, float val)
{
	comb->damp1 = val;
	comb->damp2 = 1 - val;
}


float comb_getdamp(comb* comb)
{
	return comb->damp1;
}


void comb_setfeedback(comb* comb, float val)
{
	comb->feedback = val;
}


float comb_getfeedback(comb* comb)
{
	return comb->feedback;
}


float comb_process(comb* comb, float input) 
{ 
	float output; 

	output = comb->buffer[comb->bufidx]; 
	comb->filterstore = (output * comb->damp2) + (comb->filterstore * comb->damp1); 
	comb->buffer[comb->bufidx] = input + (comb->filterstore * comb->feedback); 
	if (++comb->bufidx >= comb->bufsize) { 
		comb->bufidx = 0; 
	} 

	return output; 
} 


static void revmodel_update(revmodel_t* rev);
static void revmodel_init(revmodel_t* rev);
void set_revmodel_buffers(revmodel_t* rev, float sample_rate);


revmodel_t* rev_init(revmodel_t *rev, float sample_rate)
{
	set_revmodel_buffers(rev, sample_rate);

	/* Set default values */
	allpass_setfeedback(&rev->allpassL[0], 0.5f);
	allpass_setfeedback(&rev->allpassR[0], 0.5f);
	allpass_setfeedback(&rev->allpassL[1], 0.5f);
	allpass_setfeedback(&rev->allpassR[1], 0.5f);
	allpass_setfeedback(&rev->allpassL[2], 0.5f);
	allpass_setfeedback(&rev->allpassR[2], 0.5f);
	allpass_setfeedback(&rev->allpassL[3], 0.5f);
	allpass_setfeedback(&rev->allpassR[3], 0.5f);

	rev->gain = fixedgain;
	revmodel_set(rev,REVMODEL_SET_ALL,initialroom,initialdamp,initialwidth,initialwet);

	return rev;
}



void set_revmodel_buffers(revmodel_t* rev, float sample_rate) 
{

	float srfactor = sample_rate/44100.0f;

	comb_setbuffer(&rev->combL[0], combtuningL1*srfactor);
	comb_setbuffer(&rev->combR[0], combtuningR1*srfactor);
	comb_setbuffer(&rev->combL[1], combtuningL2*srfactor);
	comb_setbuffer(&rev->combR[1], combtuningR2*srfactor);
	comb_setbuffer(&rev->combL[2], combtuningL3*srfactor);
	comb_setbuffer(&rev->combR[2], combtuningR3*srfactor);
#if numcombs > 3
	comb_setbuffer(&rev->combL[3], combtuningL4*srfactor);
	comb_setbuffer(&rev->combR[3], combtuningR4*srfactor);
#endif
#if numcombs > 4
	comb_setbuffer(&rev->combL[4], combtuningL5*srfactor);
	comb_setbuffer(&rev->combR[4], combtuningR5*srfactor);
#endif
#if numcombs > 5
	comb_setbuffer(&rev->combL[5], combtuningL6*srfactor);
	comb_setbuffer(&rev->combR[5], combtuningR6*srfactor);
#endif
#if numcombs > 6
	comb_setbuffer(&rev->combL[6], combtuningL7*srfactor);
	comb_setbuffer(&rev->combR[6], combtuningR7*srfactor);
#endif
#if numcombs > 7
	comb_setbuffer(&rev->combL[7], combtuningL8*srfactor);
	comb_setbuffer(&rev->combR[7], combtuningR8*srfactor);
#endif
	allpass_setbuffer(&rev->allpassL[0], allpasstuningL1*srfactor);
	allpass_setbuffer(&rev->allpassR[0], allpasstuningR1*srfactor);
#if numallpasses > 1
	allpass_setbuffer(&rev->allpassL[1], allpasstuningL2*srfactor);
	allpass_setbuffer(&rev->allpassR[1], allpasstuningR2*srfactor);
#endif
#if numallpasses > 2
	allpass_setbuffer(&rev->allpassL[2], allpasstuningL3*srfactor);
	allpass_setbuffer(&rev->allpassR[2], allpasstuningR3*srfactor);
#endif
#if numallpasses > 3
	allpass_setbuffer(&rev->allpassL[3], allpasstuningL4*srfactor);
	allpass_setbuffer(&rev->allpassR[3], allpasstuningR4*srfactor);
#endif

	/* Clear all buffers */
	revmodel_init(rev);
}


static void revmodel_init(revmodel_t* rev)
{
	int i;
	for (i = 0; i < numcombs;i++) {
		comb_init(&rev->combL[i]);
		comb_init(&rev->combR[i]);
	}
	for (i = 0; i < numallpasses; i++) {
		allpass_init(&rev->allpassL[i]);
		allpass_init(&rev->allpassR[i]);
	}
}


void revmodel_reset(revmodel_t* rev)
{
	revmodel_init(rev);
}


void revmodel_process(revmodel_t* rev, float in, float *left_out, float *right_out)
{
	int i = 0;
	float outL, outR, input;

	outL = outR = 0;
	input = (in + DC_OFFSET) * rev->gain;

	/* Accumulate comb filters in parallel */
	for (i = 0; i < numcombs; i++) {
		outL += comb_process(&rev->combL[i], input);
		outR += comb_process(&rev->combR[i], input);
	}

	/* Feed through allpasses in series */
	for (i = 0; i < numallpasses; i++) {
		outL = allpass_process(&rev->allpassL[i], outL);
		outR = allpass_process(&rev->allpassR[i], outR);
	}

	outL -= DC_OFFSET;
	outR -= DC_OFFSET;

	*left_out = outL * rev->wet1 + outR * rev->wet2;
	*right_out = outR * rev->wet1 + outL * rev->wet2;
}



static void revmodel_update(revmodel_t* rev)
{
	/* Recalculate internal values after parameter change */
	int i;

	rev->wet1 = rev->wet * (rev->width / 2.0f + 0.5f);
	rev->wet2 = rev->wet * ((1.0f - rev->width) / 2.0f);

	for (i = 0; i < numcombs; i++) {
		comb_setfeedback(&rev->combL[i], rev->roomsize);
		comb_setfeedback(&rev->combR[i], rev->roomsize);
	}

	for (i = 0; i < numcombs; i++) {
		comb_setdamp(&rev->combL[i], rev->damp);
		comb_setdamp(&rev->combR[i], rev->damp);
	}
}


/**
 * Set one or more reverb parameters.
 * @param rev Reverb instance
 * @param set One or more flags from #revmodel_set_t indicating what
 *   parameters to set (#REVMODEL_SET_ALL to set all parameters)
 * @param roomsize Reverb room size
 * @param damping Reverb damping
 * @param width Reverb width
 * @param level Reverb level
 */
void revmodel_set(revmodel_t* rev, int set, float roomsize,
		float damping, float width, float level)
{
	if (set & REVMODEL_SET_ROOMSIZE)
		rev->roomsize = (roomsize * scaleroom) + offsetroom;

	if (set & REVMODEL_SET_DAMPING)
		rev->damp = damping * scaledamp;

	if (set & REVMODEL_SET_WIDTH)
		rev->width = width;

	if (set & REVMODEL_SET_LEVEL)
	{
		rev->wet = level * scalewet;
	}

	revmodel_update (rev);
}


/*
 * End
 */

