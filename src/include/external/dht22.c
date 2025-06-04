#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "dht22.h"
#include <stdio.h>
#include "include/config.h"

void dht22_init_sensor() {
    gpio_init(DHT_PIN);
    printf("DHT22 inicializado no pino GPIO%d\n", DHT_PIN);
}

/**
 * @brief Espera o pino atingir um nível específico ou até o timeout.
 * @param pin O pino GPIO.
 * @param level O nível esperado (0 ou 1).
 * @param timeout_us Duração máxima da espera em microssegundos.
 * @return true se o nível foi atingido, false se ocorreu timeout.
 */
static bool wait_for_level(uint pin, bool level, uint32_t timeout_us) {
    absolute_time_t timeout_time = make_timeout_time_us(timeout_us);
    while (gpio_get(pin) != level) {
        if (time_reached(timeout_time)) {
            return false; // Timeout
        }
    }
    return true;
}

/**
 * @brief Envia o sinal de start para o sensor DHT.
 */
void dht_send_start_signal() {
    gpio_set_dir(DHT_PIN, GPIO_OUT);
    gpio_put(DHT_PIN, 0);
    busy_wait_ms(2); // Mantenha baixo por 1-10ms (datasheet sugere >1ms)
    gpio_put(DHT_PIN, 1);
    busy_wait_us(30); // Mantenha alto por 20-40µs
    gpio_set_dir(DHT_PIN, GPIO_IN);
    // A linha deve ser puxada para cima pelo resistor de pull-up externo
}

/**
 * @brief Verifica a resposta do sensor DHT após o sinal de start.
 * @return true se a resposta for válida, false caso contrário.
 */
bool dht_check_response() {
    // DHT responde: BAIXO por ~80us, depois ALTO por ~80us
    if (!wait_for_level(DHT_PIN, 0, DHT_TIMEOUT_US)) return false; // Espera DHT puxar BAIXO
    if (!wait_for_level(DHT_PIN, 1, DHT_TIMEOUT_US)) return false; // Espera DHT puxar ALTO
    // DHT então puxa BAIXO novamente para iniciar a transmissão de dados
    if (!wait_for_level(DHT_PIN, 0, DHT_TIMEOUT_US)) return false;
    return true;
}

/**
 * @brief Lê um byte de dados do sensor DHT.
 * @param byte Ponteiro para armazenar o byte lido.
 * @return true se a leitura foi bem-sucedida, false em caso de timeout.
 */
bool dht_read_byte(uint8_t *byte) {
    *byte = 0;
    for (int i = 0; i < 8; i++) {
        // 1. Espera o pulso BAIXO (~50µs) que precede cada bit terminar (linha ir para ALTO).
        if (!wait_for_level(DHT_PIN, 1, 60)) return false; // Timeout um pouco > 50µs

        // 2. Mede a duração do pulso ALTO para determinar se é 0 ou 1.
        //    Bit '0': ~26-28µs HIGH
        //    Bit '1': ~70µs HIGH
        busy_wait_us(DHT_PULSE_SAMPLE_TIME_US); // Espera um tempo para amostrar

        if (gpio_get(DHT_PIN)) { // Se ainda estiver ALTO, é um bit '1'
            *byte |= (1 << (7 - i));
            // Espera o resto do pulso ALTO de '1' terminar (linha ir para BAIXO)
            if (!wait_for_level(DHT_PIN, 0, 60)) return false; // Timeout para o resto do pulso '1'
        } else {
            // Já está BAIXO, então era um bit '0'.
            // O próximo wait_for_level (no início do loop ou na próxima chamada de byte)
            // pegará o início do próximo pulso BAIXO.
        }
    }
    return true;
}

/**
 * @brief Realiza a leitura completa de temperatura e umidade do sensor DHT.
 * @param temperature Ponteiro para armazenar a temperatura.
 * @param humidity Ponteiro para armazenar a umidade.
 * @return true se a leitura e o checksum forem válidos, false caso contrário.
 */
bool dht_read_data(float *temperature, float *humidity) {
    uint8_t data[5] = {0};

    dht_send_start_signal();

    if (!dht_check_response()) {
        // printf("Falha na resposta do DHT\n");
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (!dht_read_byte(&data[i])) {
            // printf("Falha lendo byte %d\n", i);
            return false;
        }
    }

    // Verificar Checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (checksum != data[4]) {
        // printf("Erro de Checksum. Lido: %02X, Calculado: %02X\n", data[4], checksum);
        return false;
    }

    // Decodificar Umidade e Temperatura
    *humidity = (float)((data[0] << 8) | data[1]) / 10.0f;
    float temp_val = (float)(((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
    if (data[2] & 0x80) { // Bit de sinal para temperatura negativa
        temp_val = -temp_val;
    }
    *temperature = temp_val;

    return true;
}

