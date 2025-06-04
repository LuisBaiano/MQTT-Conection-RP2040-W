#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"

// Inicializa os pinos GPIO dos botões e configura as interrupções
void buttons_init(void);
// Verifica se o botão foi pressionado desde a última chamada e consome o evento
bool button_a_pressed(void);
bool button_b_pressed(void);

#endif // BUTTONS_H