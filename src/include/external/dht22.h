#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>
#include <stdint.h>

void dht22_init_sensor(void);
static bool wait_for_level(uint pin, bool level, uint32_t timeout_us);
void dht_send_start_signal(void);
bool dht_check_response(void);
bool dht_read_byte(uint8_t *byte);
bool dht_read_data(float *temperature, float *humidity);

#endif // DHT22_H