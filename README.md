# Pico W MQTT: Sistema de Monitoramento e Controle Agrícola

## Índice

- [Pico W MQTT: Sistema de Monitoramento e Controle Agrícola](#pico-w-mqtt-sistema-de-monitoramento-e-controle-agrícola)
  - [Índice](#índice)
  - [Objetivos do Projeto](#objetivos-do-projeto)
  - [Descrição do Projeto](#descrição-do-projeto)
  - [Funcionalidades Implementadas](#funcionalidades-implementadas)
  - [Requisitos Técnicos Atendidos](#requisitos-técnicos-atendidos)
  - [Como Executar](#como-executar)
    - [Requisitos de Hardware](#requisitos-de-hardware)
    - [Requisitos de Software](#requisitos-de-software)
    - [Configuração](#configuração)
    - [Compilação e Gravação](#compilação-e-gravação)
    - [Configuração do Broker MQTT](#configuração-do-broker-mqtt)
    - [Testes](#testes)
  - [Estrutura do Código](#estrutura-do-código)
  - [Melhorias Futuras (Opcional)](#melhorias-futuras-opcional)
  - [Vídeo de Demonstração (Espaço Reservado)](#vídeo-de-demonstração-espaço-reservado)

## Objetivos do Projeto

* Desenvolver um dispositivo IoT robusto na Raspberry Pi Pico W para monitoramento e controle agrícola.
* Coletar dados ambientais em tempo real: temperatura e umidade do ar (DHT22) e luminosidade ambiente (LDR).
* Simular o monitoramento do nível de reservatório de água usando um joystick.
* Implementar comunicação bidirecional com um broker MQTT para publicação de dados e recepção de comandos.
* Controlar atuadores (irrigação simulada e iluminação artificial) com base em comandos recebidos via MQTT.
* Fornecer feedback visual local usando um display OLED (dados dos sensores, IP, status MQTT) e uma matriz de LEDs (status dos atuadores).
* Utilizar um LED RGB para indicação do status do sistema (Wi-Fi, conexão MQTT).
* Consolidar o entendimento da programação C para sistemas embarcados, conectividade Wi-Fi (CYW43439), protocolo MQTT (LwIP) e interação com diversos periféricos (ADC, GPIO, I2C).

## Descrição do Projeto

Este projeto transforma a Raspberry Pi Pico W em um "Nó Agrícola Inteligente". O sistema se conecta a uma rede Wi-Fi, estabelece uma conexão com um broker MQTT e, em seguida, monitora continuamente as condições ambientais usando sensores reais. Esses dados são publicados periodicamente em tópicos MQTT específicos. Simultaneamente, o Pico W se inscreve em tópicos MQTT para receber comandos para controlar sistemas simulados de irrigação e iluminação artificial.

**Fluxo de Dados e Controle:**

1. **Coleta de Dados dos Sensores (Reais e Simulados):**
   * **DHT22 (Sensor Externo):** Coleta leituras reais de **Temperatura do Ar** e **Umidade do Ar**.
   * **LDR (Sensor Externo):** Mede a **Luminosidade Ambiente** (percentual).
   * **Joystick (ADC):** Simula o **Nível do Reservatório de Água**, permitindo entrada interativa.
2. **Comunicação MQTT:**
   * **Publicação:** Dados dos sensores (temperatura, umidade, luminosidade, nível do reservatório) são publicados em tópicos MQTT designados (ex: `pico_w/client_id/sensors/temperature_air`).
   * **Inscrição (Subscribe):** O dispositivo se inscreve em tópicos de comando (ex: `pico_w/client_id/actuators/irrigation/set`, `pico_w/client_id/actuators/light/set`).
   * **Relato de Estado:** O estado atual dos atuadores (Ligado/Desligado) é publicado em tópicos de estado (ex: `pico_w/client_id/actuators/irrigation/state`) após um comando ser processado ou na inicialização.
3. **Feedback Local no Pico W:**
   * **Display OLED (I2C):** Mostra o status da conexão Wi-Fi (endereço IP), status da conexão MQTT e leituras de sensores e estados dos atuadores atualizados periodicamente.
   * **Matriz de LEDs (WS2812 via PIO):** Representa visualmente o estado dos atuadores:
     * Ícone de gota d'água quando "Irrigação" está LIGADA.
     * Ícone de lâmpada quando "Luz Artificial" está LIGADA.
     * Matriz é limpa quando os atuadores estão DESLIGADOS.
   * **LED RGB (GPIO/PWM):** Indica o status geral do sistema:
     * (ex: Amarelo: Conectando Wi-Fi)
     * (ex: Ciano: Conectando MQTT)
     * (ex: Verde: Wi-Fi & MQTT Conectados)
     * (ex: Vermelho: Erro - Falha no Wi-Fi ou MQTT)
   * **LED Integrado do Pico W:** Pisca se o Wi-Fi está conectado, mas o MQTT não; totalmente apagado quando o MQTT está conectado.

**Tratamento de Comandos MQTT:**
A função `mqtt_incoming_data_cb` processa mensagens recebidas nos tópicos inscritos. Ela analisa o payload (ex: "ON" ou "OFF"), atualiza a variável de estado do atuador correspondente (`rele_irrigacao`, `rele_luz`), aciona a exibição na matriz de LEDs e publica o novo estado de volta para o broker MQTT.

## Funcionalidades Implementadas

✅ Conexão Wi-Fi (Modo Estação) com tratamento de erros.
✅ Implementação de Cliente MQTT (LwIP) para conexão a um broker.
✅ Geração de Client ID único baseado no ID da placa Pico.
✅ Callback de status da conexão.
✅ Resolução DNS para o hostname do broker MQTT.
✅ Publicação de Dados dos Sensores via MQTT:
✅ Temperatura do Ar (DHT22).
✅ Umidade do Ar (DHT22).
✅ Luminosidade (LDR).
✅ Nível Simulado do Reservatório (Joystick).
✅ Inscrição em Tópicos MQTT para Controle de Atuadores:
✅ Controle de Irrigação (LIGA/DESLIGA).
✅ Controle de Luz Artificial (LIGA/DESLIGA).
✅ Publicação dos Estados dos Atuadores via MQTT (loop de feedback).
✅ Leitura de Sensores em Tempo Real:
✅ DHT22 para temperatura e umidade.
✅ LDR para luminosidade.
✅ Simulação Interativa:
✅ Joystick para entrada do nível do reservatório.
✅ Feedback Visual Local:
✅ Display OLED: IP Wi-Fi, status MQTT, valores dos sensores, estados dos atuadores.
✅ Matriz de LEDs: Ícones para atuadores ativos (gota d'água, lâmpada).
✅ LED RGB: Status do sistema (conectividade Wi-Fi, MQTT, erros).
✅ LED Integrado: Indicação de tentativa de conexão MQTT.
✅ Código Estruturado: Design modular com funções específicas para MQTT e drivers de periféricos.
✅ Logging de Depuração: Saída serial (`printf`) para status e depuração.
✅ Configuração: Configurações de Wi-Fi e MQTT centralizadas (ex: em `mqtt_config.h`).


## Requisitos Técnicos Atendidos

*(Esta seção normalmente se alinha com critérios específicos de um curso ou projeto. Abaixo está uma interpretação geral.)*

1.  **Funcionalidade Principal:** O sistema conecta-se com sucesso ao Wi-Fi e a um broker MQTT, publica dados dos sensores e responde a comandos MQTT para controlar atuadores simulados.
2.  **Integração de Periféricos:** Demonstra uso e integração eficazes de DHT22, LDR, Joystick (ADC), OLED (I2C), Matriz de LEDs (PIO) e LED RGB.
3.  **Comunicação de Rede:** Implementa conectividade Wi-Fi robusta e operações de cliente MQTT (conectar, publicar, inscrever, callbacks) usando LwIP.
4.  **Qualidade do Código:** Código organizado em funções lógicas e potencialmente módulos, com nomes claros e comentários que auxiliam no entendimento.
5.  **Princípios de Sistemas Embarcados:** Utiliza operações temporizadas não bloqueantes, callbacks para eventos assíncronos e gerencia o estado do dispositivo de forma eficaz.

## Como Executar

### Requisitos de Hardware

*   Raspberry Pi Pico W.
*   Cabo Micro-USB para alimentação e programação.
*   Sensores:
    *   Sensor de temperatura e umidade DHT22.
    *   LDR (Resistor Dependente de Luz) com um resistor pull-down apropriado (ex: 10kΩ).
*   Periféricos:
    *   Display OLED SSD1306 (I2C).
    *   Matriz de LEDs WS2812.
    *   Joystick Analógico.
    *   LED RGB de Cátodo Comum ou Anodo Comum (e resistores limitadores de corrente).
*   Protoboard e fios jumpers.
*   Acesso a uma rede Wi-Fi (2.4 GHz).
*   Um Broker MQTT (ex: Mosquitto instalado localmente, HiveMQ Cloud, Adafruit IO).

### Requisitos de Software

*   **Pico SDK** (ex: v1.5.1 ou compatível).
*   **CMake** (versão 3.13 ou superior).
*   **ARM GCC Toolchain** (ex: `arm-none-eabi-gcc`).
*   **VS Code** com as extensões "CMake Tools" e "Cortex-Debug" (ou "Pico-W-Go") (recomendado).
*   Um **Terminal Serial** (ex: Minicom, PuTTY, terminal integrado do VS Code) para visualizar a saída do `printf`.
*   Uma **Ferramenta Cliente MQTT** (ex: MQTT Explorer, `mosquitto_pub`/`mosquitto_sub`) para testes.

### Configuração

1.  **Credenciais Wi-Fi:**
    *   Edite `main.c` (ou um `wifi_config.h` dedicado, se você criar um):
        ```c
        #define WIFI_SSID "SUA_REDE_WIFI"
        #define WIFI_PASSWORD "SUA_SENHA_WIFI"
        ```
2.  **Detalhes do Broker MQTT:**
    *   Edite `mqtt_config.h`:
        ```c
        #define MQTT_SERVER "endereco_ou_ip_do_seu_broker_mqtt" // ex: "test.mosquitto.org" ou IP
        #define MQTT_PORT 1883 // Ou a porta do seu broker (1883 para não criptografado, 8883 para SSL)
        #define MQTT_USERNAME "seu_usuario_mqtt" // "" se não houver autenticação
        #define MQTT_PASSWORD "sua_senha_mqtt"   // "" se não houver autenticação
        #define MQTT_CLIENT_ID_PREFIX "PicoW_AgroNode" // Prefixo para o ID de cliente único
        // Defina seus tópicos MQTT (exemplos):
        #define MQTT_TOPIC_TEMP_AR "sensors/temperature_air"
        #define MQTT_TOPIC_UMID_AR "sensors/humidity_air"
        #define MQTT_TOPIC_LUMINOSIDADE "sensors/luminosity"
        #define MQTT_TOPIC_RESERVATORIO "sensors/reservoir_level"
        #define MQTT_TOPIC_IRRIGACAO_SET "actuators/irrigation/set"
        #define MQTT_TOPIC_IRRIGACAO_STATE "actuators/irrigation/state"
        #define MQTT_TOPIC_LUZ_SET "actuators/light/set"
        #define MQTT_TOPIC_LUZ_STATE "actuators/light/state"
        ```
        *Nota: O código geralmente constrói tópicos completos como `MQTT_CLIENT_ID_PREFIX/id_da_placa/SUFIXO_DO_TOPICO_MQTT`. Ajuste as definições se sua função `mqtt_get_full_topic` se comportar de forma diferente.*

3.  **Definições de Pinos de Hardware:**
    *   Garanta que todas as definições de pinos em `include/config.h` (ou seu arquivo de configuração de hardware) correspondam à sua fiação física para DHT22, LDR, Joystick, OLED (SDA/SCL), Matriz de LEDs e LED RGB.

### Compilação e Gravação

1.  **Configurar Diretório de Build:**
    ```bash
    mkdir build
    cd build
    ```
2.  **Executar CMake e Make:**
    ```bash
    cmake ..
    make -j$(nproc) # Ou simplesmente 'make'
    ```
3.  **Gravar no Pico W:**
    *   Segure o botão BOOTSEL do seu Pico W enquanto o conecta via USB (ou enquanto o reinicia).
    *   Ele aparecerá como um dispositivo de armazenamento em massa (RPI-RP2).
    *   Arraste e solte o arquivo `.uf2` gerado (ex: `nome_do_seu_projeto.uf2` do diretório `build`) nesta unidade.
    *   O Pico W reiniciará e executará o novo firmware.

### Configuração do Broker MQTT

*   Certifique-se de que seu broker MQTT esteja em execução e acessível pela mesma rede que o Pico W.
*   Se estiver usando autenticação, verifique se o nome de usuário e a senha em `mqtt_config.h` estão corretos.

### Testes

1.  **Monitor Serial:**
    *   Abra um terminal serial conectado à porta COM do Pico W (baud rate normalmente 115200).
    *   Observe os logs para conexão Wi-Fi, atribuição de endereço IP, tentativas de conexão MQTT e status.
2.  **Ferramenta Cliente MQTT (ex: MQTT Explorer):**
    *   Conecte-se ao seu broker MQTT.
    *   **Inscreva-se** nos tópicos que seu Pico W está publicando (ex: `PicoW_AgroNode/<seu_pico_id>/sensors/#` ou tópicos específicos de sensores, e `PicoW_AgroNode/<seu_pico_id>/actuators/+/state`). Você deve ver os dados dos sensores aparecendo.
    *   **Publique** mensagens nos tópicos de comando nos quais o Pico W está inscrito:
        *   Tópico: `PicoW_AgroNode/<seu_pico_id>/actuators/irrigation/set` Payload: `ON` ou `OFF`
        *   Tópico: `PicoW_AgroNode/<seu_pico_id>/actuators/light/set` Payload: `ON` ou `OFF`
    *   Observe o display OLED e a matriz de LEDs no Pico W reagindo a esses comandos.
    *   Verifique se o Pico W publica o novo estado nos tópicos `/state`.
3.  **Interagir com os Sensores:**
    *   Cubra/descubra o LDR.
    *   Sopre no DHT22 (com cuidado).
    *   Mova o joystick.
    *   Observe as mudanças refletidas no monitor serial, display OLED e publicações MQTT.

## Estrutura do Código


.
├── include/
│   ├── config.h            # Definições de pinos de hardware, configurações globais
│   ├── mqtt_config.h       # Broker MQTT, tópicos, credenciais
│   ├── display.h           # Cabeçalhos para funções do display OLED
│   ├── external/           # Cabeçalhos dos drivers de sensores externos
│   │   ├── dht22.h
│   │   └── ldr.h
│   ├── joystick.h          # (Se você tiver um módulo de joystick separado)
│   ├── led_matrix.h        # (Se você tiver um módulo de matriz de LEDs separado)
│   └── rgb_led.h           # (Se você tiver um módulo de LED RGB separado)
├── src/                    # Arquivos fonte C (se você dividir a implementação)
│   ├── display.c
│   ├── external/
│   │   ├── dht22.c
│   │   └── ldr.c
│   ├── joystick.c
│   ├── led_matrix.c
│   └── rgb_led.c
├── main.c                  # Lógica principal da aplicação, Wi-Fi, configuração MQTT, loop
├── lwipopts.h              # Configuração da pilha LwIP (se customizado)
├── CMakeLists.txt          # Script de build do CMake
└── pico_sdk_import.cmake   # Importação padrão do Pico SDK

## Vídeo de Demonstração (Espaço Reservado)

[[Link para o Seu Vídeo de Demonstração]]
