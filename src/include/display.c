#include "display.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"

/**
  * @brief Desenha um ícone de casa no display OLED.
  *
  * @param ssd Ponteiro para a estrutura de controle do display SSD1306.
  */
void draw_house_icon(ssd1306_t *ssd, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    if (width < 4 || height < 4) return;
    uint8_t roof_peak_y = y;
    uint8_t wall_top_y = y + (height / 2);
    uint8_t wall_bottom_y = y + height - 1;
    ssd1306_vline(ssd, x, wall_top_y, wall_bottom_y, true);
    ssd1306_vline(ssd, x + width - 1, wall_top_y, wall_bottom_y, true);
    ssd1306_hline(ssd, x, x + width - 1, wall_bottom_y, true);
    ssd1306_line(ssd, x, wall_top_y, x + (width / 2), roof_peak_y, true);
    ssd1306_line(ssd, x + width - 1, wall_top_y, x + (width / 2), roof_peak_y, true);
}

void display_message(ssd1306_t *ssd, const char *line1, const char *line2) {
    ssd1306_fill(ssd, false); // Limpa buffer antes de desenhar
    ssd1306_rect(ssd, 0, 0, 127, 63, true, false);
    if (line1) ssd1306_draw_string(ssd, line1, 4, 0);
    if (line2) ssd1306_draw_string(ssd, line2, 4, 64 / 2 - 8);
    ssd1306_send_data(ssd); // Envia para o display físico
}

/**
  * @brief Inicializa a comunicação I2C e o display OLED SSD1306.
  *        Configura os pinos SDA e SCL, inicializa o periférico I2C e
  *        envia os comandos de configuração para o display.
  *
  * @param ssd Ponteiro para a estrutura de controle do display SSD1306 a ser inicializada.
  */
 void display_init(ssd1306_t *ssd) {
     // Inicializa I2C na porta e velocidade definidas
     i2c_init(I2C_PORT, 400 * 1000);
     // Configura os pinos GPIO para a função I2C
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
     // Habilita resistores de pull-up internos para os pinos I2C
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
     // Inicializa a estrutura do driver SSD1306 com os parâmetros do display
    ssd1306_init(ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
     // Envia a sequência de comandos de configuração para o display
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
    printf("Display inicializado.\n");
}

/**
  * @brief Exibe uma tela de inicialização no display OLED.
  *        Mostra um texto de título e o ícone do semáforo por alguns segundos.
  *
  * @param ssd Ponteiro para a estrutura de controle do display SSD1306.
  */
 void display_startup_screen(ssd1306_t *ssd) {
     // Limpa o display
    ssd1306_fill(ssd, false);
     // Calcula posições aproximadas para centralizar o texto
    uint8_t center_x_approx = ssd->width / 2;
    uint8_t start_y = 8;
    uint8_t line_height = 10; // Espaçamento vertical entre linhas

     // Define as strings a serem exibidas
    const char *line1 = "EMBARCATECH";
    const char *line2 = "PROJETO";
    const char *line3 = "MQTT";
    ssd1306_draw_string(ssd, line1, center_x_approx - (strlen(line1)*8)/2, start_y);
    ssd1306_draw_string(ssd, line2, center_x_approx - (strlen(line2)*8)/2, start_y + line_height);
    ssd1306_draw_string(ssd, line3, center_x_approx - (strlen(line3)*8)/2, start_y + 2*line_height);
  
    uint8_t icon_x = center_x_approx - 24 / 2;
    // Desenha o semáforo
    draw_house_icon(ssd, icon_x, 106, 24, 20);
    ssd1306_send_data(ssd);
    // Mantém a tela visível por um tempo
    sleep_ms(2500);
    // Limpa o display após a tela de inicialização
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}