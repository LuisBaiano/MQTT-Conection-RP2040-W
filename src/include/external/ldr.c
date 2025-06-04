#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h" // Adicionado para PWM
#include "include/config.h"


// Função para mapear um valor de uma faixa para outra
long mapeamento(long x, long in_min, long in_max, long out_min, long out_max) {
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

void ldr_init_sensor(){
    adc_gpio_init(LDR_ADC_PIN);
    adc_select_input(LDR_ADC_INPUT);
    printf("Sensor LDR inicializado\n");
}

float ldr_read_percentage() {
    adc_select_input(LDR_ADC_INPUT);
    uint16_t raw_value = adc_read();

    const uint16_t ldr_raw_min_para_escuro_total = 200;  
    const uint16_t ldr_raw_max_para_luz_total   = 3800; 

    float luz_percent = 0;
    if (raw_value <= ldr_raw_min_para_escuro_total) {
        luz_percent = 0.0f;
    } else if (raw_value >= ldr_raw_max_para_luz_total) {
        luz_percent = 100.0f;
    } else {
        luz_percent = mapeamento(raw_value, ldr_raw_min_para_escuro_total, ldr_raw_max_para_luz_total, 0, 100);
    }
    return luz_percent;
}
