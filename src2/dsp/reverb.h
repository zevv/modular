

#define numcombs 5
#define numallpasses 4

typedef struct _allpass allpass;
typedef struct _revmodel_t revmodel_t;
typedef struct _comb comb;



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

