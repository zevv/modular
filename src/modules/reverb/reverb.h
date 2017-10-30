#ifndef reverb_h
#define reverb_h

#define numcombs 6
#define numallpasses 4

typedef struct _allpass allpass;
typedef struct _revmodel_t revmodel_t;
typedef struct _comb comb;

typedef enum
{
	REVMODEL_SET_ROOMSIZE       = 1 << 0,
	REVMODEL_SET_DAMPING        = 1 << 1,
	REVMODEL_SET_WIDTH          = 1 << 2,
	REVMODEL_SET_LEVEL          = 1 << 3
} revmodel_set_t;

#define REVMODEL_SET_ALL      0x0F


struct _allpass {
	float feedback;
	float buffer[576];
	int bufsize;
	int bufidx;
};


struct _comb {
	float feedback;
	float filterstore;
	float damp1;
	float damp2;
	float buffer[1640];
	int bufsize;
	int bufidx;
};

struct _revmodel_t {
	float roomsize;
	float damp;
	float wet, wet1, wet2;
	float width;
	float gain;
	
	comb combL[numcombs];
	comb combR[numcombs];

	allpass allpassL[numallpasses];
	allpass allpassR[numallpasses];
};

revmodel_t* rev_init(revmodel_t *rev, float sample_rate);
void revmodel_process(revmodel_t* rev, float in, float *left_out, float *right_out);
void revmodel_set(revmodel_t* rev, int set, float roomsize,
		float damping, float width, float level);

#endif

