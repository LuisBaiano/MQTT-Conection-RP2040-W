#ifndef LDR_H
#define LDR_H

#include "pico/stdlib.h"

void ldr_init_sensor(void);
float ldr_read_percentage(void);

#endif // LDR_H