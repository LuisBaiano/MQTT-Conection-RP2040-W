#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "led_matrix.h"
#include "config.h"
#include "pico/stdlib.h"
#include "led_matrix.pio.h"
#include <math.h>
#include <string.h>

static PIO pio_instance = pio0;
static uint pio_sm = 0;
static uint32_t pixel_buffer[MATRIX_SIZE]; 

typedef struct { 
    float r; 
    float g; 
    float b; 
} ws2812b_color_t;

//define os leds apagados com cor devido o controle de brilho através do pio.
static const ws2812b_color_t COLOR_BLACK = { 0.0f, 0.0f, 0.0f };
static const ws2812b_color_t COLOR_RED   = { 0.25f, 0.0f, 0.0f };
static const ws2812b_color_t COLOR_GREEN = { 0.0f, 0.25f, 0.0f };
static const ws2812b_color_t COLOR_BLUE_WATER  = { 0.0f, 0.2f, 0.5f }; // Um azul para a gota de água
static const ws2812b_color_t COLOR_YELLOW_LIGHT = { 0.5f, 0.4f, 0.0f }; // Um amarelo/laranja para luz

//posição real dos leds da matrix na BitDogLab
static const uint8_t Leds_Matrix_postion[MATRIX_DIM][MATRIX_DIM] = {
    {   24,    23,    22,    21,    20 }, 
    {   15,    16,    17,    18,    19 }, 
    {   14,    13,    12,    11,    10 }, 
    {    5,     6,     7,     8,     9 }, 
    {    4,     3,     2,     1,     0 }  
};

//determinar a posição de cada led para fazer a manipulação
static inline int posicao_leds(int lin, int col) {
    if (lin >= 1 && lin <= MATRIX_DIM &&
        col >= 1 && col <= MATRIX_DIM) {
        return Leds_Matrix_postion[lin - 1][col - 1];
    }
    return -1;
}

// faz as modoficações para definir o brilho como float
static inline uint32_t color_to_pio_format(ws2812b_color_t color, float brightness) {
    float r = fmaxf(0.0f, fminf(1.0f, color.r * brightness));
    float g = fmaxf(0.0f, fminf(1.0f, color.g * brightness));
    float b = fmaxf(0.0f, fminf(1.0f, color.b * brightness));
    unsigned char R_val = (unsigned char)(r * 255.0f);
    unsigned char G_val = (unsigned char)(g * 255.0f);
    unsigned char B_val = (unsigned char)(b * 255.0f);
    return ((uint32_t)(G_val) << 24) | ((uint32_t)(R_val) << 16) | ((uint32_t)(B_val) << 8);
}

static void update_matrix() {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        pio_sm_put_blocking(pio_instance, pio_sm, pixel_buffer[i]);
    }
    busy_wait_us(50);
}

// Helper to set a pixel using PHYSICAL coordinates (1-based row/col)
static void led_active_position(int lin, int col, uint32_t color) {
    int logical_index = posicao_leds(lin, col);
    if (logical_index != -1) {
        pixel_buffer[logical_index] = color;
    }
}

//inicia a matriz
void led_matrix_init() {
    uint offset = pio_add_program(pio_instance, &led_matrix_program);
    led_matrix_program_init(pio_instance, pio_sm, offset, MATRIX_WS2812_PIN);
    led_matrix_clear();
}

//apaga os leds da matriz
void led_matrix_clear() {
    uint32_t pio_black = color_to_pio_format(COLOR_BLACK, 1.0f);
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        pixel_buffer[i] = pio_black;
    }
    update_matrix();
}

/**
 * @brief Desenha um ícone de gota d'água na matriz de LEDs.
 *        Usa a cor COLOR_BLUE_WATER.
 */
void led_matrix_draw_water_drop() {
    uint32_t pio_water_color = color_to_pio_format(COLOR_BLUE_WATER, 0.4f); // Brilho 1.0f aqui usa o brilho já embutido na cor

    // Linha 1 (topo da gota)
    led_active_position(1, 3, pio_water_color);
    // Linha 2
    led_active_position(2, 2, pio_water_color);
    led_active_position(2, 3, pio_water_color);
    led_active_position(2, 4, pio_water_color);
    // Linha 3 (meio, mais largo)
    led_active_position(3, 1, pio_water_color);
    led_active_position(3, 2, pio_water_color);
    led_active_position(3, 3, pio_water_color);
    led_active_position(3, 4, pio_water_color);
    led_active_position(3, 5, pio_water_color);
    // Linha 4
    led_active_position(4, 2, pio_water_color);
    led_active_position(4, 3, pio_water_color);
    led_active_position(4, 4, pio_water_color);
    // Linha 5 (base da gota)
    led_active_position(5, 3, pio_water_color);

    update_matrix();
}

/**
 * @brief Desenha um ícone de sol na matriz de LEDs.
 *        Usa a cor COLOR_YELLOW_LIGHT.
 */
void led_matrix_draw_light_icon() {
    uint32_t pio_light_color = color_to_pio_format(COLOR_YELLOW_LIGHT, 0.4f);

    // Centro
    led_active_position(3, 3, pio_light_color);
    // "Raios" ou corpo da lâmpada
    led_active_position(2, 3, pio_light_color); // Acima
    led_active_position(4, 3, pio_light_color); // Abaixo
    led_active_position(3, 2, pio_light_color); // Esquerda
    led_active_position(3, 4, pio_light_color); // Direita

    // Diagonais (opcional, para parecer mais "sol")
    led_active_position(2, 2, pio_light_color);
    led_active_position(2, 4, pio_light_color);
    led_active_position(4, 2, pio_light_color);
    led_active_position(4, 4, pio_light_color);

    // Pode adicionar mais pixels para um ícone mais cheio, por exemplo:
    // led_active_position(1, 3, pio_light_color);
    // led_active_position(5, 3, pio_light_color);
    // led_active_position(3, 1, pio_light_color);
    // led_active_position(3, 5, pio_light_color);

    update_matrix();
}