// LwIP e MQTT includes
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "pico/unique_id.h" // Para o Client ID
#include "lwip/altcp_tls.h" 

// ----- DEFINES MQTT -----
#define MQTT_SERVER         "192.168.101.3" //
// #define MQTT_PORT           1883 
#define MQTT_USERNAME       "luisbaiano"      // Deixe "" se não houver autenticação
#define MQTT_PASSWORD       "135469"        // Deixe "" se não houver autenticação
#define MQTT_CLIENT_ID_PREFIX "PicoAgro" // Será concatenado com o ID único da placa

#define MQTT_TOPIC_BASE                "picoagro" // Base para todos os tópicos
#define MQTT_TOPIC_TEMP_AR             MQTT_TOPIC_BASE "/temperature_air"
#define MQTT_TOPIC_UMID_AR             MQTT_TOPIC_BASE "/humidity_air"
#define MQTT_TOPIC_LUMINOSIDADE        MQTT_TOPIC_BASE "/luminosity"
#define MQTT_TOPIC_RESERVATORIO        MQTT_TOPIC_BASE "/reservoir_level"

#define MQTT_TOPIC_IRRIGACAO_SET       MQTT_TOPIC_BASE "/irr/set"  // Para receber comandos
#define MQTT_TOPIC_IRRIGACAO_STATE     MQTT_TOPIC_BASE "/irr/state"// Para publicar estado
#define MQTT_TOPIC_LUZ_SET             MQTT_TOPIC_BASE "/light/set"       // Para receber comandos
#define MQTT_TOPIC_LUZ_STATE           MQTT_TOPIC_BASE "/light/state"     // Para publicar estado

#define MQTT_PUBLISH_INTERVAL_MS (2500) // Publicar dados dos sensores a cada 10 segundos
#define MQTT_KEEP_ALIVE_S 60
#define MQTT_QOS 1
#define MQTT_RETAIN 0 // Para dados de sensores. Pode ser 1 para estados de relé.