#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "credentials.h"

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "pico/unique_id.h"

// ----- DEFINES MQTT -----
#define MQTT_SERVER         MQTT_SERVER_CREDENTIALS
#define MQTT_USERNAME       MQTT_USERNAME_CREDENTIALS
#define MQTT_PASSWORD       MQTT_PASSWORD_CREDENTIALS // Senha MQTT
#define MQTT_CLIENT_ID_PREFIX "PicoAgro"  // Será concatenado com o ID único da placa

// Tópicos de Publicação (Dados dos Sensores enviados PELO PICO)
#define MQTT_TOPIC_BASE                "picoagro" // Base para todos os tópicos
#define MQTT_TOPIC_TEMP_AR             MQTT_TOPIC_BASE "/temperature_air"
#define MQTT_TOPIC_UMID_AR             MQTT_TOPIC_BASE "/humidity_air"
#define MQTT_TOPIC_LUMINOSIDADE        MQTT_TOPIC_BASE "/luminosity"
#define MQTT_TOPIC_RESERVATORIO        MQTT_TOPIC_BASE "/reservoir_level"

// Tópicos de Estado (Publicados PELO PICO para feedback dos atuadores)
#define MQTT_TOPIC_IRRIGACAO_STATE     MQTT_TOPIC_BASE "/irrigation/state" // Corrigido
#define MQTT_TOPIC_LUZ_STATE           MQTT_TOPIC_BASE "/light/state"

// Configurações de Publicação/Conexão
#define MQTT_PUBLISH_INTERVAL_MS  (3000) // Publicar dados dos sensores a cada 3 segundos
#define MQTT_KEEP_ALIVE_S         60      // Keep alive em segundos
#define MQTT_QOS                  1       // Qualidade de Serviço para publicações/inscrições
#define MQTT_RETAIN_SENSOR_DATA   0       // Flag Retain para dados de sensores (geralmente 0)
#define MQTT_RETAIN_STATE_DATA    1       // Flag Retain para estados de relé (geralmente 1)

// Limites para buffers internos no mqtt_state
#define MQTT_CLIENT_ID_MAX_LEN  32
#define MQTT_MAX_TOPIC_LEN        128     // Comprimento máximo de string para nome de tópico
#define MQTT_MAX_PAYLOAD_LEN      64      // Comprimento máximo para payload (ON/OFF e floats pequenos cabem)

#endif // MQTT_CONFIG_H