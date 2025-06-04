#ifndef RGB_LED_H
#define RGB_LED_H

#include "pico/stdlib.h"
#include "config.h" 

// luz de status do sistema
#define RGB_OFF     0, 0, 0
#define RGB_ERROR   1, 0, 0 
#define RGB_CONNECTING 1, 1, 0 
#define RGB_MQTT_OK   0, 1 ,1

void rgb_led_init(void);
void rgb_led_set(bool r, bool g, bool b);

#endif // RGB_LED_H