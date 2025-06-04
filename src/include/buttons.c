#include "include/buttons.h"
#include "include/config.h" 
#include "include/debouncer.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"

// Flags para os botões
static volatile bool g_button_a_pressed_flag = false;
static volatile bool g_button_b_pressed_flag = false; // NOVA FLAG para Botão B

// Timers para Debounce
static uint32_t last_a_press_time = 0;
static uint32_t last_b_press_time = 0;

// Função de callback para interrupção GPIO (Trata Botão A e B)
static void button_irq_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) { // Verifica borda de descida
        if (gpio == BUTTON_A_PIN) {
            if (check_debounce(&last_a_press_time, DEBOUNCE_TIME_US)) {
                g_button_a_pressed_flag = true;
                // printf("DEBUG: Botao A Pressionado (Flag SET)\n");
            }
        } else if (gpio == BUTTON_B_PIN) {
            if (check_debounce(&last_b_press_time, DEBOUNCE_TIME_US)) {
                g_button_b_pressed_flag = true;
                // printf("DEBUG: Botao B Pressionado (Flag SET)\n");
            }
        }
    }
}

// Inicializa os pinos GPIO dos botões E configura IRQs
void buttons_init(void) { // Renomeado de init_buttons_gpio para consistência

    // Botão A
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_irq_callback);

    // Botão B
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_irq_callback);
}

// Verifica se o botão A foi pressionado
bool button_a_pressed(void) {
    if (g_button_a_pressed_flag) {
        g_button_a_pressed_flag = false; // Reseta a flag (consome o evento)
        return true;
    }
    return false;
}

// Verifica se o botão B foi pressionado
bool button_b_pressed(void) { // NOVA FUNÇÃO
    if (g_button_b_pressed_flag) {
        g_button_b_pressed_flag = false; // Reseta a flag (consome o evento)
        return true;
    }
    return false;
}
