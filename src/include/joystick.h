#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h" // Para uint16_t

void joystick_init(void);
uint16_t read_adc(uint adc_channel);
float joytisck_read_percentage(void);

#endif // JOYSTICK_Hs