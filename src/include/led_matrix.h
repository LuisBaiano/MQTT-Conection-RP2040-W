#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

void led_matrix_init();
void led_matrix_clear();
// FUNÇÕES NOVAS A SEREM IMPLEMENTADAS em led_matrix.c
void led_matrix_draw_water_drop(void);
void led_matrix_draw_light_icon(void);

// Suas funções existentes, se for usar de alguma forma:
// void led_matrix_ped_walk(void);
// void led_matrix_ped_dont_walk(bool flash_state);

#endif // LED_MATRIX_H