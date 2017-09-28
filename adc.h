#ifndef adc_h
#define adc_h

void adc_init(void);
uint32_t adc_read(float val[8]);
void adc_tick(void);

#endif

