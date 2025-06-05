#ifndef LDR_H
#define LDR_H

#include "pico/stdlib.h"

void ldr_init_sensor(void);
bool ldr_light_detected(void);

#endif // LDR_H