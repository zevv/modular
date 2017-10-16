#ifndef adc_h
#define adc_h

void adc_init(void);
uint32_t adc_read(volatile int32_t *val);
void adc_tick(void);

#endif

