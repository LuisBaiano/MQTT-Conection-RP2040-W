#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "pico/bootrom.h"   
#include "hardware/i2c.h"  
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "hardware/timer.h"

// controle dos perifericos
#include "hardware/pio.h"
#include "buzzer.h"  
#include "buttons.h"   
#include "joystick.h"     
#include "rgb_led.h"    
#include "led_matrix.h"

//Sensores reais

// --- Configurações DHT22 ---
#define DHT_PIN 17 
#define DHT_TIMEOUT_US 120  //testar e mudar com a integração   
#define DHT_PULSE_SAMPLE_TIME_US 35 

// --- Configurações LDR ---
#define LDR_ADC_PIN 28  //
#define LDR_ADC_INPUT 2 // ADC input 0 corresponde a GP26
#define ADC_VREF 3.3f
#define ADC_RANGE (1 << 12) // 4096 (0-4095)

#define LED_RED_PIN     13 
#define LED_GREEN_PIN   11 
#define LED_BLUE_PIN    12
#define LED_PWM_WRAP 255     // Valor de wrap para PWM do LED RGB 
#define LED_PIN CYW43_WL_GPIO_LED_PIN   // Onboard

#define JOYSTICK_X_PIN 26 
#define JOYSTICK_Y_PIN 27 
#define JOYSTICK_X_ADC_CHANNEL 0 
#define JOYSTICK_Y_ADC_CHANNEL 1 
#define ADC_CENTER 2048  

// Botões
#define BUTTON_A_PIN    5 
#define BUTTON_B_PIN    6 

// Buzzer
#define BUZZER_PIN_1 10
#define BUZZER_PIN_2 21

// LED Matrix
#define MATRIX_WS2812_PIN 7
#define MATRIX_SIZE       25
#define MATRIX_DIM        5

// Display
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define DISPLAY_ADDR 0x3C
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64


// --- tempos de delay das tarefas ---
#define DEBOUNCE_TIME_US    20000
#define DHT_READ_TIME_MS    2500
#define LDR_READ_TIME_MS    500
#define JOY_READ_TIME_MS    500
#define OLED_UPDATE_INTERVAL_MS 1500

// --- tempos do buzzer ---
#define BUZZER_ALERT_FREQ      1500 // F#6 (mais agudo e rápido)
#define BUZZER_ALERT_ON_MS     150


#endif // HARDWARE_CONFIG_H 