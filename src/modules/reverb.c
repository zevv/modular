#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "module.h"
#include "os/printd.h"

#define numcombs 8
#define numallpasses 4
#define	fixedgain 0.007f
#define scalewet 3.0f
#define scaledamp 1.0f
#define scaleroom 0.28f
#define offsetroom 0.7f
#define initialroom 0.5f
#define initialdamp 0.2f
#define initialwet 1
#define initialdry 0
#define initialwidth 1
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

typedef struct _revmodel_t revmodel_t;


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

typedef struct _allpass allpass;
typedef struct _comb comb;

#define ARRAY(t, n) calloc(sizeof(t), n)
#define NEW(t) calloc(sizeof(t), 1)

struct _allpass {
	float feedback;
	float *buffer;
	int bufsize;
	int bufidx;
};

void allpass_init(allpass* allpass);
void allpass_setfeedback(allpass* allpass, float val);
float allpass_getfeedback(allpass* allpass);

void allpass_setbuffer(allpass* allpass, int size)
{
	allpass->bufidx = 0;
	allpass->buffer = ARRAY(float,size);
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


#define allpass_process(_allpass, _input) \
{ \
	float output; \
	float bufout; \
	bufout = _allpass.buffer[_allpass.bufidx]; \
	output = bufout-_input; \
	_allpass.buffer[_allpass.bufidx] = _input + (bufout * _allpass.feedback); \
	if (++_allpass.bufidx >= _allpass.bufsize) { \
		_allpass.bufidx = 0; \
	} \
	_input = output; \
}

/*  float allpass_process(allpass* allpass, float input) */
/*  { */
/*    float output; */
/*    float bufout; */
/*    bufout = allpass->buffer[allpass->bufidx]; */
/*    undenormalise(bufout); */
/*    output = -input + bufout; */
/*    allpass->buffer[allpass->bufidx] = input + (bufout * allpass->feedback); */
/*    if (++allpass->bufidx >= allpass->bufsize) { */
/*      allpass->bufidx = 0; */
/*    } */
/*    return output; */
/*  } */


struct _comb {
	float feedback;
	float filterstore;
	float damp1;
	float damp2;
	float *buffer;
	int bufsize;
	int bufidx;
};

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
	comb->buffer = ARRAY(float,size);
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


#define comb_process(_comb, _input, _output) \
{ \
	float _tmp = _comb.buffer[_comb.bufidx]; \
	_comb.filterstore = (_tmp * _comb.damp2) + (_comb.filterstore * _comb.damp1); \
	_comb.buffer[_comb.bufidx] = _input + (_comb.filterstore * _comb.feedback); \
	if (++_comb.bufidx >= _comb.bufsize) { \
		_comb.bufidx = 0; \
	} \
	_output += _tmp; \
}

/* float comb_process(comb* comb, float input) */
/* { */
/*    float output; */

/*    output = comb->buffer[comb->bufidx]; */
/*    undenormalise(output); */
/*    comb->filterstore = (output * comb->damp2) + (comb->filterstore * comb->damp1); */
/*    undenormalise(comb->filterstore); */
/*    comb->buffer[comb->bufidx] = input + (comb->filterstore * comb->feedback); */
/*    if (++comb->bufidx >= comb->bufsize) { */
/*      comb->bufidx = 0; */
/*    } */

/*    return output; */
/* } */



struct _revmodel_t {
	float roomsize;
	float damp;
	float wet, wet1, wet2;
	float width;
	float gain;
	/*
	   The following are all declared inline
	   to remove the need for dynamic allocation
	   with its subsequent error-checking messiness
	   */
	/* Comb filters */
	comb combL[numcombs];
	comb combR[numcombs];
	/* Allpass filters */
	allpass allpassL[numallpasses];
	allpass allpassR[numallpasses];
};

static void revmodel_update(revmodel_t* rev);
static void revmodel_init(revmodel_t* rev);
void set_revmodel_buffers(revmodel_t* rev, float sample_rate);


revmodel_t* new_revmodel(float sample_rate)
{
	revmodel_t* rev;
	rev = NEW(revmodel_t);
	if (rev == NULL) {
		return NULL;
	}

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
	comb_setbuffer(&rev->combL[3], combtuningL4*srfactor);
	comb_setbuffer(&rev->combR[3], combtuningR4*srfactor);
	comb_setbuffer(&rev->combL[4], combtuningL5*srfactor);
	comb_setbuffer(&rev->combR[4], combtuningR5*srfactor);
	comb_setbuffer(&rev->combL[5], combtuningL6*srfactor);
	comb_setbuffer(&rev->combR[5], combtuningR6*srfactor);
	comb_setbuffer(&rev->combL[6], combtuningL7*srfactor);
	comb_setbuffer(&rev->combR[6], combtuningR7*srfactor);
	comb_setbuffer(&rev->combL[7], combtuningL8*srfactor);
	comb_setbuffer(&rev->combR[7], combtuningR8*srfactor);
	allpass_setbuffer(&rev->allpassL[0], allpasstuningL1*srfactor);
	allpass_setbuffer(&rev->allpassR[0], allpasstuningR1*srfactor);
	allpass_setbuffer(&rev->allpassL[1], allpasstuningL2*srfactor);
	allpass_setbuffer(&rev->allpassR[1], allpasstuningR2*srfactor);
	allpass_setbuffer(&rev->allpassL[2], allpasstuningL3*srfactor);
	allpass_setbuffer(&rev->allpassR[2], allpasstuningR3*srfactor);
	allpass_setbuffer(&rev->allpassL[3], allpasstuningL4*srfactor);
	allpass_setbuffer(&rev->allpassR[3], allpasstuningR4*srfactor);

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

	/* The original Freeverb code expects a stereo signal and 'input'
	 * is set to the sum of the left and right input sample. Since
	 * this code works on a mono signal, 'input' is set to twice the
	 * input sample. */
	input = (2.0f * in + DC_OFFSET) * rev->gain;

	/* Accumulate comb filters in parallel */
	for (i = 0; i < numcombs; i++) {
		comb_process(rev->combL[i], input, outL);
		comb_process(rev->combR[i], input, outR);
	}
	/* Feed through allpasses in series */
	for (i = 0; i < numallpasses; i++) {
		allpass_process(rev->allpassL[i], outL);
		allpass_process(rev->allpassR[i], outR);
	}

	/* Remove the DC offset */
	outL -= DC_OFFSET;
	outR -= DC_OFFSET;

	/* Calculate output REPLACING anything already there */
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


/************************************************************************/


struct mod_reverb {
	revmodel_t *verb;
	bool enable;
	float level;
	float damp;
	float roomsize;
};


static void *fn_new(uint32_t srate)
{
	struct mod_reverb *mo;
	mo = malloc(sizeof *mo);
	mo->verb = new_revmodel(srate * 0.5);

	return mo;
}


static void fn_audio(struct module *mod, float *sig)
{
	struct mod_reverb *mo = mod->data;
	if(mo->enable) {
		float out1, out2;
		revmodel_process(mo->verb, sig[0], &out1, &out2);
		sig[1] = sig[0] * (1.0 - mo->level) + out1 * mo->level;
		sig[2] = sig[0] * (1.0 - mo->level) + out2 * mo->level;
	} else {
		sig[1] = sig[0];
		sig[2] = sig[0];
	}
}


static void fn_control(struct module *mod, size_t idx, float val)
{
	struct mod_reverb *mo = mod->data;
	if(idx == 0) {
		mo->enable = val;
	}
	if(idx == 1) {
		mo->roomsize = val;
	}
	if(idx == 2) {
		mo->level = val;
	}
	if(idx == 3) {
		mo->damp = val ? 0.2 : 0.7;
	}
	revmodel_set(mo->verb, REVMODEL_SET_ROOMSIZE | REVMODEL_SET_DAMPING,
			mo->roomsize, mo->damp, 0, 0);
	//float damping, float width, float level);
}


MODULE_REGISTER {
	.name = "reverb",
		.fn_new = fn_new,
		.fn_audio = fn_audio,
		.fn_control = fn_control,
};


