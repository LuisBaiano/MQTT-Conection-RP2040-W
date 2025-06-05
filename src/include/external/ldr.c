#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "include/config.h" // Garanta que LDR_DO_PIN está definido aqui


/**
 * @brief Inicializa o pino GPIO para leitura do sensor LDR com saída digital (DO).
 */
void ldr_init_sensor() {
    gpio_init(LDR_DO_PIN);             // Inicializa o pino GPIO
    gpio_set_dir(LDR_DO_PIN, GPIO_IN); // Configura o pino como entrada
}

/**
 * @brief Lê o estado da saída digital (DO) do módulo sensor LDR.
 *
 * @return bool true se uma condição for detectada (ex: luz acima do limiar), false caso contrário.
 *              A interpretação exata (luz/escuro) depende do módulo.
 */
bool ldr_light_detected() { // Ou você pode nomear ldr_is_dark_detected() dependendo do seu uso
    bool pin_state = gpio_get(LDR_DO_PIN);


    return pin_state;
}
