#ifndef adc_h
#define adc_h

void adc_init(void);
float adc_read(int channel);
void adc_tick(void);

#endif

