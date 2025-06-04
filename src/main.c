#include "mqtt_config.h"
#include "credentials.h"
#include "include/config.h"
#include "include/display.h"
#include "include/external/dht22.h"
#include "include/external/ldr.h"

// ----- GLOBAIS (COMO NO SEU CÓDIGO) -----
static ssd1306_t ssd_global;
static char pico_ip_address[20] = "N/A";

static float temp_ar = 0.0f;
static float umid_ar = 0.0f;
static float luminosidade = 0.0f;
static float reservatorio = 0.0f; // Simulado pelo joystick
static bool rele_irrigacao = false;
static bool rele_luz = false;

// ----- Credenciais WIFI -----
#define WIFI_SSID         WIFI_SSID_CREDENTIALS
#define WIFI_PASSWORD     WIFI_PASSWORD_CREDENTIALS

static uint32_t last_dht_read_time = 0;
static uint32_t last_ldr_read_time = 0;
static uint32_t last_joy_read_time = 0;
static uint32_t last_mqtt_publish_time = 0;
static uint32_t last_oled_update_time = 0;

// ----- ESTRUTURA E GLOBAL MQTT -----
typedef struct MQTT_CLIENT_DATA_T_ {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t client_info;
    ip_addr_t server_addr;
    char unique_client_id[MQTT_CLIENT_ID_MAX_LEN];
    bool connected;
    char current_topic[MQTT_MAX_TOPIC_LEN];
    char current_payload[MQTT_MAX_PAYLOAD_LEN];
    uint16_t current_payload_len;
} MQTT_CLIENT_DATA_T;
static MQTT_CLIENT_DATA_T mqtt_state;

// ----- PROTÓTIPOS MQTT -----
static void mqtt_init_client_info(MQTT_CLIENT_DATA_T* state);
static void mqtt_start_client(MQTT_CLIENT_DATA_T* state);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_pub_request_cb(void *arg, err_t err);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_dns_found_cb(const char *hostname, const ip_addr_t *ipaddr, void *arg);
static void mqtt_do_subscribe_topics(MQTT_CLIENT_DATA_T* state);
static void mqtt_publish_float_data(MQTT_CLIENT_DATA_T* state, const char* topic_name, float value);
static void mqtt_publish_string_data(MQTT_CLIENT_DATA_T* state, const char* topic_name, const char* value_str);
static void mqtt_publish_rele_state(MQTT_CLIENT_DATA_T* state, const char* rele_topic_name, bool is_on);

// ----- FUNÇÃO PRINCIPAL (main) -----
int main() {
    stdio_init_all();
    sleep_ms(1000);
    adc_init();

    printf("Inicializando perifericos...\n");
    buttons_init();
    dht22_init_sensor();
    ldr_init_sensor();
    joystick_init();
    rgb_led_init();    // Para o LED RGB de status
    led_matrix_init();
    display_init(&ssd_global);

    rgb_led_set(RGB_OFF); // Usando defines do SEU rgb_led.h (ex: 0,0,0)
    display_startup_screen(&ssd_global);
    display_message(&ssd_global, "Sistema OK", "Iniciando WiFi...");

    // ----- CONEXÃO WI-FI -----
    printf("WiFi: Inicializando cyw43...\n");
    rgb_led_set(RGB_CONNECTING); // Amarelo/Laranja (ex: 1,1,0 do seu rgb_led.h)

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) {
        printf("ERRO FATAL: cyw43_arch_init falhou\n");
        rgb_led_set(RGB_ERROR); // Vermelho (ex: 1,0,0 do seu rgb_led.h)
        display_message(&ssd_global, "ERRO FATAL", "WiFi Init Falhou");
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
        rgb_led_set(RGB_ERROR); // Vermelho
        display_message(&ssd_global, "WiFi ERRO", "Falha Conexao");
        cyw43_arch_gpio_put(LED_PIN, 1);
        return -1;
    }
    printf("WiFi: Conectado com sucesso!\n");
    cyw43_arch_gpio_put(LED_PIN, 0);

    if (netif_default) {
        strncpy(pico_ip_address, ipaddr_ntoa(netif_ip4_addr(netif_default)), sizeof(pico_ip_address) - 1);
        pico_ip_address[sizeof(pico_ip_address) - 1] = '\0';
        display_message(&ssd_global, "WiFi OK!", pico_ip_address);
    } else {
        display_message(&ssd_global, "WiFi OK!", "Sem IP Addr");
    }
    sleep_ms(1000);

    // ----- INICIALIZAÇÃO DO CLIENTE MQTT -----
    printf("MQTT: Inicializando cliente...\n");
    display_message(&ssd_global, pico_ip_address, "MQTT Client...");
    mqtt_init_client_info(&mqtt_state);

    printf("MQTT: Resolvendo DNS para broker '%s'...\n", MQTT_SERVER);
    cyw43_arch_lwip_begin();
    err_t dns_err = dns_gethostbyname(MQTT_SERVER, &mqtt_state.server_addr, mqtt_dns_found_cb, &mqtt_state);
    cyw43_arch_lwip_end();

    if (dns_err == ERR_OK) {
        printf("MQTT: Endereco IP do broker: %s\n", ipaddr_ntoa(&mqtt_state.server_addr));
        mqtt_start_client(&mqtt_state);
    } else if (dns_err == ERR_INPROGRESS) {
        printf("MQTT: Resolucao DNS em progresso...\n");
    } else {
        printf("ERRO: Falha DNS broker MQTT (%d).\n", dns_err);
        rgb_led_set(RGB_ERROR); // Vermelho
        display_message(&ssd_global, pico_ip_address, "ERRO DNS MQTT");
    }

    // ----- LOOP PRINCIPAL -----
     printf("Entrando no loop principal.\n");
    while (true) {
        cyw43_arch_poll();

        // Leitura dos Sensores (DHT, LDR, Joystick para reservatório)
        if (time_us_32() - last_dht_read_time > (DHT_READ_TIME_MS * 1000)) {
            // Use suas variáveis globais corretas para temp_ar e umid_ar
            dht_read_data(&temp_ar, &umid_ar); // Atualiza temp_ar e umid_ar
            last_dht_read_time = time_us_32();
        }
        if (time_us_32() - last_ldr_read_time > (LDR_READ_TIME_MS * 1000)) {
            luminosidade = ldr_read_percentage();
            last_ldr_read_time = time_us_32();
        }
        if (time_us_32() - last_joy_read_time > (JOY_READ_TIME_MS * 1000)) {
            uint16_t joy_val = read_adc(JOYSTICK_X_ADC_CHANNEL); // Ou o eixo que simula reservatório
            reservatorio = (joy_val / 4095.0f) * 100.0f; // Joystick simula 0-100%
            last_joy_read_time = time_us_32();
        }

        // --- CONTROLE LOCAL DOS "RELÉS" E MATRIZ PELOS BOTÕES A e B ---
        bool estado_irrigacao_mudou = false;
        bool estado_luz_mudou = false;

        if (button_a_pressed()) { // Botão A controla Irrigação
            rele_irrigacao = !rele_irrigacao;
            estado_irrigacao_mudou = true;
            printf("Botao A: Irrigacao -> %s\n", rele_irrigacao ? "ON" : "OFF");
        }

        if (button_b_pressed()) { // Botão B controla Luz Artificial
            rele_luz = !rele_luz;
            estado_luz_mudou = true;
            printf("Botao B: Luz Artificial -> %s\n", rele_luz ? "ON" : "OFF");
        }

        // Atualiza a matriz de LEDs com base nos estados dos relés
        if (rele_irrigacao) {
            led_matrix_draw_water_drop();
        } else if (rele_luz) { // Só mostra luz se irrigação estiver OFF
            led_matrix_draw_light_icon();
        } else { // Ambos OFF
            led_matrix_clear();
        }

        // Publica o novo estado se mudou e MQTT está conectado
        if (mqtt_state.connected) {
            if (estado_irrigacao_mudou) {
                mqtt_publish_rele_state(&mqtt_state, MQTT_TOPIC_IRRIGACAO_STATE, rele_irrigacao);
            }
            if (estado_luz_mudou) {
                mqtt_publish_rele_state(&mqtt_state, MQTT_TOPIC_LUZ_STATE, rele_luz);
            }
        }

        // Publicação periódica dos dados dos sensores via MQTT
        if (mqtt_state.connected && (time_us_32() - last_mqtt_publish_time > (MQTT_PUBLISH_INTERVAL_MS * 1000))) {
            // printf("MQTT PUB: T:%.1f U:%.1f L:%.1f R:%.1f\n", temp_ar, umid_ar, luminosidade, reservatorio);
            mqtt_publish_float_data(&mqtt_state, MQTT_TOPIC_TEMP_AR, temp_ar);
            mqtt_publish_float_data(&mqtt_state, MQTT_TOPIC_UMID_AR, umid_ar);

            char val_str_pub[16];

            if (luminosidade > 75.0f) { 
            strcpy(val_str_pub, "ALTA");
            } else if (luminosidade > 25.0f) {
                strcpy(val_str_pub, "MEDIA");
            } else {
                strcpy(val_str_pub, "BAIXA");
            }
            mqtt_publish_string_data(&mqtt_state, MQTT_TOPIC_LUMINOSIDADE, val_str_pub);

            if (reservatorio > 75.0f) {
                strcpy(val_str_pub, "ALTO");
            } else if (reservatorio > 25.0f) {
                strcpy(val_str_pub, "MEDIO");
            } else {
                strcpy(val_str_pub, "BAIXO");
            }
            mqtt_publish_string_data(&mqtt_state, MQTT_TOPIC_RESERVATORIO, val_str_pub);
            
            // Também publica o estado dos relés periodicamente, caso a publicação por mudança falhe ou para novos inscritos
            mqtt_publish_rele_state(&mqtt_state, MQTT_TOPIC_IRRIGACAO_STATE, rele_irrigacao);
            mqtt_publish_rele_state(&mqtt_state, MQTT_TOPIC_LUZ_STATE, rele_luz);
            
            last_mqtt_publish_time = time_us_32();
        }

        // LED Onboard do Pico W para status MQTT
        if (netif_is_up(netif_default) && !mqtt_state.connected && cyw43_is_initialized(&cyw43_state)){ // Adicionado cyw43_is_initialized
            cyw43_arch_gpio_put(LED_PIN, (time_us_32() / 25000) % 2); // Pisca
        } else if (mqtt_state.connected) {
            cyw43_arch_gpio_put(LED_PIN, 0); // Apagado se MQTT OK
        } // Se WiFi cair, LED_PIN fica como estava ou desliga (se !netif_is_up)
        
        sleep_ms(50); // Ajuste a frequência do loop principal conforme necessário
    }
    // return 0; // Inatingível
}

// ----- IMPLEMENTAÇÕES DAS FUNÇÕES MQTT -----
static void mqtt_init_client_info(MQTT_CLIENT_DATA_T* state) {
    memset(state, 0, sizeof(MQTT_CLIENT_DATA_T));
    state->connected = false;
    char board_id_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(board_id_str, sizeof(board_id_str));
    snprintf(state->unique_client_id, MQTT_CLIENT_ID_MAX_LEN, "%s-%s", MQTT_CLIENT_ID_PREFIX, board_id_str);

    state->client_info.client_id = state->unique_client_id;
    state->client_info.client_user = (strlen(MQTT_USERNAME) > 0) ? MQTT_USERNAME : NULL;
    state->client_info.client_pass = (strlen(MQTT_PASSWORD) > 0) ? MQTT_PASSWORD : NULL;
    state->client_info.keep_alive = MQTT_KEEP_ALIVE_S;
    state->client_info.will_topic = NULL; 
    state->client_info.will_msg = NULL;
    state->client_info.will_qos = 0;
    state->client_info.will_retain = 0;
}

static void mqtt_dns_found_cb(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (ipaddr != NULL) {
        state->server_addr = *ipaddr;
        printf("MQTT: DNS Resolvido para %s -> %s\n", hostname, ipaddr_ntoa(&state->server_addr));
        mqtt_start_client(state);
    } else {
        printf("ERRO: MQTT DNS falhou para %s\n", hostname);
        rgb_led_set(RGB_ERROR);
        if (ssd_global.width > 0) display_message(&ssd_global, pico_ip_address, "ERRO DNS Broker");
    }
}

static void mqtt_start_client(MQTT_CLIENT_DATA_T* state) {
    state->mqtt_client_inst = mqtt_client_new();
    if (state->mqtt_client_inst == NULL) { 
        printf("ERRO: Falha ao criar instancia MQTT.\n");
        return;
    }
    printf("MQTT: Tentando conectar ao broker %s:%d ID:'%s'...\n", ipaddr_ntoa(&state->server_addr), MQTT_PORT, state->client_info.client_id);
    if (ssd_global.width > 0) display_message(&ssd_global, pico_ip_address, "Conect MQTT...");

    cyw43_arch_lwip_begin();
    err_t err = mqtt_client_connect(state->mqtt_client_inst, &state->server_addr, MQTT_PORT,
                                    mqtt_connection_cb, state, &state->client_info);
    cyw43_arch_lwip_end();

    if (err != ERR_OK) {
        printf("ERRO: mqtt_client_connect falhou (%d)\n", err);
        rgb_led_set(RGB_ERROR);
        if (ssd_global.width > 0) display_message(&ssd_global, pico_ip_address, "Falha MQTT Conn");
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT: Conectado com sucesso ao broker!\n");
        rgb_led_set(RGB_MQTT_OK); // (Assume que RGB_MQTT_OK está definido)
        if (ssd_global.width > 0) display_message(&ssd_global, pico_ip_address, "MQTT Conectado!");
        state->connected = true;

        // Publica estado inicial dos relés (controlados localmente pelos botões)
        mqtt_publish_rele_state(state, MQTT_TOPIC_IRRIGACAO_STATE, rele_irrigacao);
        mqtt_publish_rele_state(state, MQTT_TOPIC_LUZ_STATE, rele_luz);
        last_mqtt_publish_time = 0; // Força publicação dos dados dos sensores
    } else {
        printf("MQTT: Conexao falhou ou terminou. Status: %d\n", status);
        rgb_led_set(RGB_ERROR);
        if (ssd_global.width > 0) display_message(&ssd_global, pico_ip_address, "MQTT Falhou");
        state->connected = false;
    }
}

static void mqtt_pub_request_cb(void *arg, err_t err) {
    if (err != ERR_OK) {
        printf("ERRO: Publicacao MQTT falhou (err: %d)\n", err);
    }
    else { printf("MQTT: Publicacao bem-sucedida.\n"); }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("MQTT RX HDR (IGNORADO/OUTRO): Topico '%.*s'\n", (int)tot_len, topic);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("MQTT RX DATA (IGNORADO/OUTRO): Payload '%.*s'\n", len, data);
}

static void mqtt_do_subscribe_topics(MQTT_CLIENT_DATA_T* state) {

    printf("MQTT: Configurando subscricoes (se houver)...\n");
    cyw43_arch_lwip_begin();

    cyw43_arch_lwip_end();
    printf("MQTT: Subscricoes configuradas (ou nenhuma necessaria).\n");
}

// mqtt_publish_float_data: Permanece igual.
static void mqtt_publish_float_data(MQTT_CLIENT_DATA_T* state, const char* topic_name, float value) {
    if (!state->connected || !state->mqtt_client_inst) return;
    char payload_str[16];
    snprintf(payload_str, sizeof(payload_str), "%.1f", value);
    cyw43_arch_lwip_begin();
    mqtt_publish(state->mqtt_client_inst, topic_name, payload_str, strlen(payload_str),
                MQTT_QOS, MQTT_RETAIN_SENSOR_DATA, mqtt_pub_request_cb, state);
    cyw43_arch_lwip_end();
}

static void mqtt_publish_string_data(MQTT_CLIENT_DATA_T* state, const char* topic_name, const char* value_str) {
    if (!state->connected || !state->mqtt_client_inst) return;
    cyw43_arch_lwip_begin();
    mqtt_publish(state->mqtt_client_inst, topic_name, value_str, strlen(value_str),
                 MQTT_QOS, MQTT_RETAIN_SENSOR_DATA,
                 mqtt_pub_request_cb, state);
    cyw43_arch_lwip_end();
}

static void mqtt_publish_rele_state(MQTT_CLIENT_DATA_T* state, const char* rele_topic_name, bool is_on) {
    if (!state->connected || !state->mqtt_client_inst) return;
    const char* payload = is_on ? "ON" : "OFF";
    cyw43_arch_lwip_begin();
    mqtt_publish(state->mqtt_client_inst, rele_topic_name, payload, strlen(payload),
                 MQTT_QOS, MQTT_RETAIN_STATE_DATA,
                 mqtt_pub_request_cb, state);
    cyw43_arch_lwip_end();
    printf("MQTT State PUB: %s -> %s\n", rele_topic_name, payload);
}