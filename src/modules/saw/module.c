#include <stdio.h>
#include <stdint.h>

#include "../../dsp/module.h"
#include "../../dsp/biquad.h"
#include "../../dsp/fir.h"
#include "osc.h"


#define double float


#define FILTER_TAP_NUM 71

static double filter_taps[FILTER_TAP_NUM] = {
  0.00002256809984183597,
  0.00013811477713547706,
  0.0004395826331894632,
  0.0010826254900978128,
  0.002220211218319197,
  0.003960821363642665,
  0.006278717478385035,
  0.008942558426983363,
  0.011479914789813072,
  0.01322205762349455,
  0.013443924960168642,
  0.011586910056785759,
  0.007509905309207128,
  0.001683100883831266,
  -0.004769220530126795,
  -0.010242896843741403,
  -0.01304852201462853,
  -0.01194753035910805,
  -0.006675528731361805,
  0.0017376229112136573,
  0.011015518076229048,
  0.018122396202611024,
  0.02012163298459432,
  0.015192959973277378,
  0.003498378229502983,
  -0.01242027845354367,
  -0.027927660795466304,
  -0.0372899609513561,
  -0.035176401229991106,
  -0.01827491715105232,
  0.013455126500933789,
  0.056309588764409245,
  0.10336292500112561,
  0.14592134600787424,
  0.17555577112133328,
  0.1861737446159248,
  0.17555577112133328,
  0.14592134600787424,
  0.10336292500112561,
  0.056309588764409245,
  0.013455126500933789,
  -0.01827491715105232,
  -0.035176401229991106,
  -0.0372899609513561,
  -0.027927660795466304,
  -0.01242027845354367,
  0.003498378229502983,
  0.015192959973277378,
  0.02012163298459432,
  0.018122396202611024,
  0.011015518076229048,
  0.0017376229112136573,
  -0.006675528731361805,
  -0.01194753035910805,
  -0.01304852201462853,
  -0.010242896843741403,
  -0.004769220530126795,
  0.001683100883831266,
  0.007509905309207128,
  0.011586910056785759,
  0.013443924960168642,
  0.01322205762349455,
  0.011479914789813072,
  0.008942558426983363,
  0.006278717478385035,
  0.003960821363642665,
  0.002220211218319197,
  0.0010826254900978128,
  0.0004395826331894632,
  0.00013811477713547706,
  0.00002256809984183597
};

static struct osc osc;
static struct fir fir1;

void mod_init(void)
{
//	sintab_init();

	osc_init(&osc, 192000);

	fir_init(&fir1, filter_taps, FILTER_TAP_NUM);

	osc_set_type(&osc, OSC_TYPE_SAW);
	osc_set_freq(&osc, 1000);
}


float prand(void)
{
        static uint32_t r = 22222;
        r = (r * 196314165) + 907633515;
        return r / (float)UINT32_MAX;
}


void mod_run(float *fin, float *fout)
{
	
	float f = fin[4] * 10000 + 10000;
	float g = fin[6] * 0.5 + 0.5;

	f = 500;

	osc_set_freq(&osc, f);

	int i;
	for(i=0; i<4; i++) {
		float v = osc_gen(&osc);
		fir_load(&fir1, v);
	}

	fout[0] = fout[1] = fir_calc(&fir1) * g;

}

#ifdef TEST_SAW

float min = 100.0;
float max = -100.0;
void main(void)
{
	mod_init();
	int i;
	float f[32];
	for(i=0; i<480000*2; i++) {
		f[4] = i / 480000.0 - 1.0;
		f[6] = 0.9;
		mod_run(f, f);
		if(f[0] < min) min = f[0];
		if(f[0] > max) max = f[0];
		fwrite(&f[0], 2, sizeof(f[0]), stdout);
	}
	fprintf(stderr, "%f %f\n", min, max);
}

#endif

/*
 * End
 */

