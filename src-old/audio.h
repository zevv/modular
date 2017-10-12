#ifndef audio_h
#define audio_h

struct audio {
	float in[4];
	float adc[8];
	float out[2];
};

extern struct audio au;

void audio_do(void);

#endif
