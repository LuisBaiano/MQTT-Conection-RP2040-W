#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/ssd1306/ssd1306.h" 

void display_init(ssd1306_t *ssd); 
void display_startup_screen(ssd1306_t *ssd);
void display_message(ssd1306_t *ssd, const char *line1, const char *line2);

#endif // DISPLAY_H