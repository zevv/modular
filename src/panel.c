#include <stdint.h>

#include "module.h"
#include "audio.h"
#include "panel.h"
#include "os/printd.h"


void panel_1_init(uint32_t srate)
{

}


void panel_2_init(uint32_t srate)
{
	struct module *mod;
	
	if(0) {

		mod = mod_new("vcf", srate);
		mod_map_sig(mod, 0, &au.in[0]);
		mod_map_sig(mod, 1, &au.out[0]);
		mod_map_pot(mod, 0, CTL_POT_LOG, &au.adc[0], 10, 10000);
		mod_map_switch(mod, 1, &au.adc[1]);
		mod_map_pot(mod, 2, CTL_POT_LIN, &au.adc[2], 0.1, 7.0);

		mod = mod_new("waveshaper", srate);
		mod_map_sig(mod, 0, &au.in[1]);
		mod_map_sig(mod, 1, &au.out[1]);
		mod_map_switch(mod, 0, &au.adc[3]);
	} else {
		mod = mod_new("reverb", srate);
		mod_map_sig(mod, 0, &au.in[1]);
		mod_map_sig(mod, 1, &au.out[0]);
		mod_map_sig(mod, 2, &au.out[1]);
		mod_map_pot(mod, 1, CTL_POT_LIN, &au.adc[0], 0, 1);
		mod_map_pot(mod, 2, CTL_POT_LIN, &au.adc[2], 0, 1);
		mod_map_switch(mod, 0, &au.adc[3]);
		mod_map_switch(mod, 3, &au.adc[1]);
	}
}


void panel_3_init(uint32_t srate)
{
	struct module *mod;

	mod = mod_new("vco", srate);
	mod_map_sig(mod, 0, &au.out[0]);

	mod_map_switch(mod, 0, &au.adc[0]);
	mod_map_pot(mod, 1, CTL_POT_LOG, &au.adc[1], 10, 1000);
	mod_map_switch(mod, 2, &au.adc[2]);
	
	mod = mod_new("vcf", srate);
	mod_map_sig(mod, 0, &au.in[0]);
	mod_map_sig(mod, 1, &au.out[1]);
}



