#include "config.h" 
#include "hardware/gpio.h"
#include "rgb_led.h" 


/**
 * @brief Inicializa os pinos do LED RGB como saídas digitais.
 *        Define os LEDs inicialmente como desligados.
 */
void rgb_led_init() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 0); // Desligado

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0); // Desligado

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 0); // Desligado

    printf("LED RGB (GPIO) inicializado.\n");
}

/**
 * @brief Define o estado (LIGADO/DESLIGADO) para os canais R, G e B do LED.
 *
 * @param r Estado do canal vermelho (true para LIGADO, false para DESLIGADO).
 * @param g Estado do canal verde (true para LIGADO, false para DESLIGADO).
 * @param b Estado do canal azul (true para LIGADO, false para DESLIGADO).
 */
void rgb_led_set(bool r, bool g, bool b) {
    gpio_put(LED_RED_PIN, r);
    gpio_put(LED_GREEN_PIN, g);
    gpio_put(LED_BLUE_PIN, b);
}
