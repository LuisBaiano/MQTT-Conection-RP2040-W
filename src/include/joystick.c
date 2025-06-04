#include "config.h" 
#include "hardware/adc.h"

/**
 * @brief Inicializa o ADC e os pinos analógicos do joystick.
 */
void joystick_init() {
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

/**
 * @brief Lê os valores dos eixos X e Y do joystick.
 *
 * @param x Ponteiro para armazenar o valor do eixo X (0-4095).
 * @param y Ponteiro para armazenar o valor do eixo Y (0-4095).
 */

uint16_t read_adc(uint adc_channel) {
    adc_select_input(adc_channel);
    sleep_us(5);
    return adc_read();
}

// Função para mapear um valor de uma faixa para outra
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    // Evita divisão por zero
    if (in_min == in_max) {
        return out_min; // Ou algum valor padrão, ou erro
    }
    long result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    
    // Clampa o resultado para a faixa de saída
    if (out_min < out_max) {
        if (result < out_min) return out_min;
        if (result > out_max) return out_max;
    } else { // Se out_min > out_max (mapeamento invertido)
        if (result > out_min) return out_min;
        if (result < out_max) return out_max;
    }
    return result;
}

float joytisck_read_percentage() {
    adc_select_input(JOYSTICK_X_ADC_CHANNEL);
    uint16_t value = adc_read();
    const uint16_t nivel_baixo = 200;  
    const uint16_t nivel_maximo   = 4000;

    float nivel_reservatorio = 0;
    if (value <= nivel_baixo) {
        nivel_reservatorio = 0.0f;
    } else if (value >= nivel_maximo) {
        nivel_reservatorio = 100.0f;
    } else {
        nivel_reservatorio = map(value, nivel_baixo, nivel_maximo, 0, 100);
    }
    return nivel_reservatorio;
}
