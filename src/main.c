#include "mqtt_config.h"
#include "include/config.h"
#include "include/display.h"
#include "include/external/dht22.h"
#include "include/external/ldr.h"

#define WIFI_SSID "CNAnet_ADRIANA"
#define WIFI_PASSWORD "vidanova"

// ----- GLOBAIS (já existentes) -----
// ----- GLOBAIS -----
static ssd1306_t ssd_global;
static char pico_ip_address[20] = "N/A";
static uint32_t last_dht_read_time = 0;

// Variáveis Globais para Simulação

static float temp_ar = 0.0f;
static float umid_ar = 0.0f;
static float luminosidade = 0.0f;
static float reservatorio = 0.0f;
static bool rele_irrigacao = false;
static bool rele_luz = false;

// ----- ESTRUTURA E GLOBAIS MQTT -----
typedef struct MQTT_CLIENT_DATA_T_ {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t client_info;
    ip_addr_t server_addr;
    char unique_client_id[32];
    bool connected;
    char current_topic[128]; // Para callbacks de incoming_publish
    char current_payload[256]; // Para callbacks de incoming_data
    uint16_t current_payload_len;
} MQTT_CLIENT_DATA_T;

static MQTT_CLIENT_DATA_T mqtt_state;
static uint32_t last_mqtt_publish_time = 0;

// ----- PROTÓTIPOS MQTT -----
static void mqtt_init_client_info(MQTT_CLIENT_DATA_T* state);
static void mqtt_start_client(MQTT_CLIENT_DATA_T* state);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_pub_request_cb(void *arg, err_t err);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_dns_found_cb(const char* hostname, const ip_addr_t* ipaddr, void* arg);
static void mqtt_do_subscribe_topics(MQTT_CLIENT_DATA_T* state);
static void mqtt_publish_float_data(MQTT_CLIENT_DATA_T* state, const char* topic_suffix, float value);
static void mqtt_publish_string_data(MQTT_CLIENT_DATA_T* state, const char* topic_suffix, const char* value_str);
static void mqtt_publish_rele_state(MQTT_CLIENT_DATA_T* state, const char* rele_topic_suffix_state, bool is_on);
static const char* mqtt_get_full_topic(MQTT_CLIENT_DATA_T* state, const char* topic_suffix, char* buffer, size_t buffer_len);

static void controlar_dispositivo_com_matriz(
    MQTT_CLIENT_DATA_T *state,
    const char *state_topic,
    bool *rele,
    bool is_on_command,
    void (*draw_icon)(void),
    const char* dispositivo_nome
) {
    if (is_on_command) {
        *rele = true;
        draw_icon(); // desenha o ícone correspondente
        printf("  Acao: %s LIGADO\n", dispositivo_nome);
    } else {
        *rele = false;
        led_matrix_clear(); // limpa a matriz
        printf("  Acao: %s DESLIGADO\n", dispositivo_nome);
    }

    // Publica novo estado no tópico /state
    char topic_buffer[128];
    mqtt_get_full_topic(state, state_topic, topic_buffer, sizeof(topic_buffer));
    const char *msg = *rele ? "On" : "Off";
    mqtt_publish(state->mqtt_client_inst, topic_buffer, msg, strlen(msg), MQTT_QOS, MQTT_RETAIN, mqtt_pub_request_cb, state);
}


// ----- FUNÇÃO PRINCIPAL-----
int main() {
    stdio_init_all();
    sleep_ms(500);
    adc_init();

    // ----- INICIALIZAÇÃO DE PERIFÉRICOS -----
    printf("Inicializando perifericos...\n");
    dht22_init_sensor();
    ldr_init_sensor();
    joystick_init();
    rgb_led_init();
    led_matrix_init();
    display_init(&ssd_global);
    display_startup_screen(&ssd_global);
    display_message(&ssd_global, "Sistema OK", "Iniciando WiFi...");

    // ----- CONEXÃO WI-FI (mantida) -----
    printf("WiFi: Inicializando cyw43...\n");
    rgb_led_set(RGB_CONNECTING);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) {
        printf("ERRO FATAL: cyw43_arch_init falhou\n");
        display_message(&ssd_global, "ERRO FATAL", "WiFi Init Falhou");
        rgb_led_set(RGB_ERROR);
        return -1;
    }
    printf("WiFi: cyw43 inicializado OK.\n");
    cyw43_arch_enable_sta_mode();
    cyw43_arch_gpio_put(LED_PIN, 0); 

    printf("WiFi: Conectando a '%s'...\n", WIFI_SSID);
    display_message(&ssd_global, "Conectando:", WIFI_SSID);
    cyw43_arch_gpio_put(LED_PIN, 1); 

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERRO: Falha ao conectar ao Wi-Fi.\n");
        rgb_led_set(RGB_ERROR);
        display_message(&ssd_global, "WiFi ERRO", "Falha Conexao");
        cyw43_arch_gpio_put(LED_PIN, 1); 
        return -1;
    }

    printf("WiFi: Conectado com sucesso!\n");
    cyw43_arch_gpio_put(LED_PIN, 0); 

    if (netif_default) {
        strncpy(pico_ip_address, ipaddr_ntoa(netif_ip4_addr(netif_default)), sizeof(pico_ip_address) - 1);
        pico_ip_address[sizeof(pico_ip_address) - 1] = '\0'; // Garantir terminação nula
        printf("IP do dispositivo: %s\n", pico_ip_address);
        display_message(&ssd_global, "WiFi OK!", pico_ip_address);
    } else {
        printf("WiFi Conectado, mas sem info de IP.\n");
        display_message(&ssd_global, "WiFi OK!", "Sem IP Addr");
    }
    sleep_ms(1000);

    // ----- INICIALIZAÇÃO DO CLIENTE MQTT (NOVO) -----
    printf("MQTT: Inicializando cliente...\n");
    display_message(&ssd_global, pico_ip_address, "MQTT Client...");
    mqtt_init_client_info(&mqtt_state);

    // Resolução de DNS para o broker MQTT
    cyw43_arch_lwip_begin(); // Necessário para operações de rede como DNS
    err_t dns_err = dns_gethostbyname(MQTT_SERVER, &mqtt_state.server_addr, mqtt_dns_found_cb, &mqtt_state);
    cyw43_arch_lwip_end();

    if (dns_err == ERR_OK) { // Se já for um IP ou cache
        printf("MQTT: Endereco IP do broker obtido diretamente: %s\n", ipaddr_ntoa(&mqtt_state.server_addr));
        mqtt_start_client(&mqtt_state);
    } else if (dns_err == ERR_INPROGRESS) {
        printf("MQTT: Resolucao DNS em progresso para %s\n", MQTT_SERVER);
        // O callback mqtt_dns_found_cb chamará mqtt_start_client
    } else {
        printf("ERRO: Falha ao iniciar resolucao DNS para o broker MQTT (%d).\n", dns_err);
        rgb_led_set(RGB_ERROR);
        display_message(&ssd_global, pico_ip_address, "ERRO DNS MQTT");
        // return -1; // Pode querer permitir que o resto funcione sem MQTT
    }


    // ----- LOOP PRINCIPAL -----
    printf("Entrando no loop principal.\n");
    uint32_t last_oled_update_time = 0;
    uint32_t last_ldr_read_time = 0;
    uint32_t last_joy_read_time = 0;
    

    while (true) {
        cyw43_arch_poll(); 

        uint16_t adc_x = read_adc(JOYSTICK_X_ADC_CHANNEL);
        uint16_t adc_y = read_adc(JOYSTICK_Y_ADC_CHANNEL);

        if (time_us_32() - last_ldr_read_time > (LDR_READ_TIME_MS * 1000)) {
            luminosidade = ldr_read_percentage();
        }

        //leitura continua do dht22 para obter a temperatura e a umidade
        if (time_us_32() - last_dht_read_time > (DHT_READ_TIME_MS * 1000)) {
            float temp_dht, hum_dht;
            printf("Lendo DHT22...\n");
            if (dht_read_data(&temp_dht, &hum_dht)) {
                temp_ar = temp_dht; // Atualiza globais com dados reais
                umid_ar = hum_dht;
                printf("DHT22 Leitura OK: Temperatura = %.1f C, Umidade = %.1f%% \n", temp_ar, umid_ar);
            } else {
                printf("Falha ao ler DHT22.\n");

            }
            last_dht_read_time = time_us_32();
        }

        if (time_us_32() - last_joy_read_time > (JOY_READ_TIME_MS * 1000)) {
            reservatorio = joytisck_read_percentage();
        }

        // O LED do Pico W pisca se não estiver conectado ao MQTT (após tentativa de conexão WiFi)
        if (netif_is_up(netif_default) && !mqtt_state.connected) {
             cyw43_arch_gpio_put(LED_PIN, (time_us_32() / 500000) % 2); // Pisca a cada 0.5s
        } else if (mqtt_state.connected) {
            cyw43_arch_gpio_put(LED_PIN, 0); // LED apagado se conectado
        }

    }

    cyw43_arch_deinit();
    return 0;
}


// ----- IMPLEMENTAÇÕES DAS FUNÇÕES MQTT -----

static const char* mqtt_get_full_topic(MQTT_CLIENT_DATA_T* state, const char* topic_suffix, char* buffer, size_t buffer_len) {
    // No momento, não estamos prefixando com client_id, mas esta função permitiria
    // snprintf(buffer, buffer_len, "%s%s", MQTT_TOPIC_BASE, topic_suffix); // Se o sufixo já inclui a /
    strncpy(buffer, topic_suffix, buffer_len -1); // Se topic_suffix já é o nome completo do tópico
    buffer[buffer_len -1] = '\0';
    return buffer;
}


static void mqtt_init_client_info(MQTT_CLIENT_DATA_T* state) {
    memset(state, 0, sizeof(MQTT_CLIENT_DATA_T)); // Limpa a estrutura
    state->connected = false;

    // Gerar Client ID único
    char board_id_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(board_id_str, sizeof(board_id_str));
    snprintf(state->unique_client_id, sizeof(state->unique_client_id), "%s_%s", MQTT_CLIENT_ID_PREFIX, board_id_str);

    state->client_info.client_id = state->unique_client_id;
    state->client_info.client_user = strlen(MQTT_USERNAME) > 0 ? MQTT_USERNAME : NULL;
    state->client_info.client_pass = strlen(MQTT_PASSWORD) > 0 ? MQTT_PASSWORD : NULL;
    state->client_info.keep_alive = MQTT_KEEP_ALIVE_S;
    state->client_info.will_topic = NULL; // Configure LWT se desejar
    state->client_info.will_msg = NULL;
    state->client_info.will_qos = 0;
    state->client_info.will_retain = 0;
    // state->client_info.tls_config = NULL; // Se usar TLS, configurar aqui
}

static void mqtt_dns_found_cb(const char* hostname, const ip_addr_t* ipaddr, void* arg) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (ipaddr) {
        state->server_addr = *ipaddr;
        printf("MQTT: DNS Resolvido para %s -> %s\n", hostname, ipaddr_ntoa(ipaddr));
        mqtt_start_client(state);
    } else {
        printf("ERRO: MQTT DNS falhou para %s\n", hostname);
        // Tentar novamente ou indicar erro no display
        display_message(&ssd_global, pico_ip_address, "ERRO DNS Broker");
    }
}

static void mqtt_start_client(MQTT_CLIENT_DATA_T* state) {
    if (state->mqtt_client_inst) { // Já existe, talvez reconectando
        mqtt_disconnect(state->mqtt_client_inst);
        // mqtt_client_free(state->mqtt_client_inst); // A API LwIP pode não ter free, verificar
        state->mqtt_client_inst = NULL;
    }

    state->mqtt_client_inst = mqtt_client_new();
    if (state->mqtt_client_inst == NULL) {
        printf("ERRO: Falha ao criar instancia MQTT.\n");
        return;
    }

    printf("MQTT: Conectando ao broker %s:%d como '%s'...\n", ipaddr_ntoa(&state->server_addr), MQTT_PORT, state->client_info.client_id);
    display_message(&ssd_global, pico_ip_address, "Conect MQTT...");

    cyw43_arch_lwip_begin();
    err_t err = mqtt_client_connect(state->mqtt_client_inst,
                                    &state->server_addr,
                                    MQTT_PORT,
                                    mqtt_connection_cb,
                                    state, // Passa o estado como argumento
                                    &state->client_info);
    cyw43_arch_lwip_end();

    if (err != ERR_OK) {
        printf("ERRO: Falha ao conectar ao MQTT broker (%d)\n", err);
        display_message(&ssd_global, pico_ip_address, "Falha MQTT Conn");
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT: Conectado com sucesso!\n");
        display_message(&ssd_global, pico_ip_address, "MQTT Conectado!");
        rgb_led_set(RGB_MQTT_OK); // Define uma cor para MQTT conectado
        state->connected = true;
        
        // Configurar callbacks para mensagens recebidas
        mqtt_set_inpub_callback(client,
                                mqtt_incoming_publish_cb,
                                mqtt_incoming_data_cb,
                                arg);
        // Inscrever-se nos tópicos
        mqtt_do_subscribe_topics(state);

        // Publicar estado inicial dos relés
        mqtt_publish_rele_state(state, MQTT_TOPIC_IRRIGACAO_STATE, rele_irrigacao);
        mqtt_publish_rele_state(state, MQTT_TOPIC_LUZ_STATE, rele_luz);
        last_mqtt_publish_time = 0; // Força a publicação dos dados dos sensores logo
    } else {
        printf("MQTT: Conexao falhou/terminou, status: %d\n", status);
        display_message(&ssd_global, pico_ip_address, "MQTT Falhou");
        rgb_led_set(RGB_ERROR); // Ou uma cor específica para erro MQTT
        state->connected = false;
        // Tentar reconectar? LwIP pode fazer isso automaticamente se keep-alive falhar.
        // Ou agendar uma tentativa de reconexão.
        // Por agora, simples. Se usar LWT, o broker saberá da desconexão.
    }
}

static void mqtt_pub_request_cb(void *arg, err_t err) {
    // MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (err == ERR_OK) {
        // printf("MQTT: Publicacao bem-sucedida.\n");
    } else {
        printf("ERRO: Falha na publicacao MQTT (%d).\n", err);
    }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    printf("MQTT RX HDR: Topic '%.*s', Length %lu\n", (int)tot_len, topic, tot_len); // tot_len aqui é o comprimento do tópico

    if (tot_len < sizeof(state->current_topic)) {
        strncpy(state->current_topic, topic, tot_len);
        state->current_topic[tot_len] = '\0';
    } else {
        printf("MQTT RX HDR: Nome do topico muito longo.\n");
        state->current_topic[0] = '\0';
    }
    // Limpa o payload para a próxima mensagem
    state->current_payload[0] = '\0';
    state->current_payload_len = 0;
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;

    if ((state->current_payload_len + len) < sizeof(state->current_payload)) {
        memcpy(state->current_payload + state->current_payload_len, data, len);
        state->current_payload_len += len;
        state->current_payload[state->current_payload_len] = '\0';
    } else {
        printf("MQTT_DEBUG: Payload recebido muito grande.\n");
        state->current_topic[0] = '\0'; state->current_payload[0] = '\0'; state->current_payload_len = 0;
        return;
    }

    if (flags & MQTT_DATA_FLAG_LAST) {
        printf("MQTT RX: Topic '%.*s', Payload '%.*s'\n",
               (int)strlen(state->current_topic), state->current_topic,
               state->current_payload_len, state->current_payload);

        char topic_buffer_cmp[128];
        bool is_on_command = ((strncmp(state->current_payload, "ON", state->current_payload_len) == 0 && state->current_payload_len == 2) ||
                              (strncmp(state->current_payload, "on", state->current_payload_len) == 0 && state->current_payload_len == 2));

        // --- IRRIGAÇÃO ---
        mqtt_get_full_topic(state, MQTT_TOPIC_IRRIGACAO_SET, topic_buffer_cmp, sizeof(topic_buffer_cmp));
        if (strcmp(state->current_topic, topic_buffer_cmp) == 0) {
            printf("MQTT: Comando para Irrigacao SET ['%.*s']\n", state->current_payload_len, state->current_payload);
            controlar_dispositivo_com_matriz(state, MQTT_TOPIC_IRRIGACAO_STATE, &rele_irrigacao, is_on_command, led_matrix_draw_water_drop, "Irrigacao");
        }

        // --- LUZ ARTIFICIAL ---
        else {
            mqtt_get_full_topic(state, MQTT_TOPIC_LUZ_SET, topic_buffer_cmp, sizeof(topic_buffer_cmp));
            if (strcmp(state->current_topic, topic_buffer_cmp) == 0) {
                printf("MQTT: Comando para Luz SET ['%.*s']\n", state->current_payload_len, state->current_payload);
                controlar_dispositivo_com_matriz(state, MQTT_TOPIC_LUZ_STATE, &rele_luz, is_on_command, led_matrix_draw_light_icon, "Luz Artificial");
            } else {
                printf("DEBUG: Topico NAO RECONHECIDO para controle: '%s'\n", state->current_topic);
            }
        }

        state->current_topic[0] = '\0';
        state->current_payload[0] = '\0';
        state->current_payload_len = 0;
    }
}


static void mqtt_do_subscribe_topics(MQTT_CLIENT_DATA_T* state) {
    char topic_buffer[128];
    err_t err;

    printf("MQTT: Inscrevendo-se nos topicos...\n");
    
    // Tópico de irrigação
    mqtt_get_full_topic(state, MQTT_TOPIC_IRRIGACAO_SET, topic_buffer, sizeof(topic_buffer));
    err = mqtt_subscribe(state->mqtt_client_inst, topic_buffer, MQTT_QOS, mqtt_pub_request_cb, state); // Usa mesmo callback de pub para sub
    if(err != ERR_OK) { printf("ERRO ao inscrever-se em %s: %d\n", topic_buffer, err); }
    else { printf("Inscrito em: %s\n", topic_buffer); }

    // Tópico de luz
    mqtt_get_full_topic(state, MQTT_TOPIC_LUZ_SET, topic_buffer, sizeof(topic_buffer));
    err = mqtt_subscribe(state->mqtt_client_inst, topic_buffer, MQTT_QOS, mqtt_pub_request_cb, state);
    if(err != ERR_OK) { printf("ERRO ao inscrever-se em %s: %d\n", topic_buffer, err); }
    else { printf("Inscrito em: %s\n", topic_buffer); }
}

static void mqtt_publish_float_data(MQTT_CLIENT_DATA_T* state, const char* topic_suffix, float value) {
    if (!state->connected) return;
    char payload_str[16];
    char full_topic_str[128];
    snprintf(payload_str, sizeof(payload_str), "%.1f", value);
    mqtt_get_full_topic(state, topic_suffix, full_topic_str, sizeof(full_topic_str));
    
    // printf("MQTT PUB: %s -> %s\n", full_topic_str, payload_str); // Debug
    mqtt_publish(state->mqtt_client_inst,
                 full_topic_str,
                 payload_str,
                 strlen(payload_str),
                 MQTT_QOS,
                 MQTT_RETAIN, // Retain pode ser 0 para dados de sensores
                 mqtt_pub_request_cb,
                 state);
}

static void mqtt_publish_string_data(MQTT_CLIENT_DATA_T* state, const char* topic_suffix, const char* value_str) {
    if (!state->connected) return;
    char full_topic_str[128];
    mqtt_get_full_topic(state, topic_suffix, full_topic_str, sizeof(full_topic_str));

    // printf("MQTT PUB: %s -> %s\n", full_topic_str, value_str); // Debug
    mqtt_publish(state->mqtt_client_inst,
                 full_topic_str,
                 value_str,
                 strlen(value_str),
                 MQTT_QOS,
                 MQTT_RETAIN,
                 mqtt_pub_request_cb,
                 state);
}

static void mqtt_publish_rele_state(MQTT_CLIENT_DATA_T* state, const char* rele_topic_suffix_state, bool is_on) {
    if (!state->connected) return;
    const char* payload = is_on ? "ON" : "OFF";
    mqtt_publish_string_data(state, rele_topic_suffix_state, payload); // Usar Retain = 1 para estados é comum
    printf("MQTT State PUB: %s -> %s\n", rele_topic_suffix_state, payload);
}
