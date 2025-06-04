
# Pico W MQTT: Sistema de Monitoramento e Controle Agrícola Inteligente 🌾💧💡

[![Linguagem C](https://img.shields.io/badge/Linguagem-C-blue.svg)](https://www.iso.org/standard/74528.html)
[![Raspberry Pi Pico W](https://img.shields.io/badge/Plataforma-Raspberry%20Pi%20Pico%20W-brightgreen.svg)](https://www.raspberrypi.com/products/raspberry-pi-pico/)
[![MQTT](https://img.shields.io/badge/Protocolo-MQTT-orange.svg)](https://mqtt.org/)
[![Status do Projeto](https://img.shields.io/badge/Status-Em%20Desenvolvimento-yellowgreen.svg)](#) <!-- Ou Concluído, Arquivado, etc. -->

**Um sistema IoT robusto construído na Raspberry Pi Pico W para monitoramento agrícola em tempo real e controle local de atuadores, utilizando MQTT para publicação de dados e estados.**

---

## 📖 Índice

- [Pico W MQTT: Sistema de Monitoramento e Controle Agrícola Inteligente 🌾💧💡](#pico-w-mqtt-sistema-de-monitoramento-e-controle-agrícola-inteligente-)
  - [📖 Índice](#-índice)
  - [🎯 Objetivos do Projeto](#-objetivos-do-projeto)
  - [📝 Descrição do Projeto](#-descrição-do-projeto)
    - [📊 Fluxo de Dados e Controle](#-fluxo-de-dados-e-controle)
    - [📡 Comunicação MQTT e Feedback Local](#-comunicação-mqtt-e-feedback-local)
  - [✨ Funcionalidades Implementadas](#-funcionalidades-implementadas)
  - [🛠️ Requisitos Técnicos Atendidos](#️-requisitos-técnicos-atendidos)
  - [🚀 Como Executar](#-como-executar)
    - [🔩 Requisitos de Hardware](#-requisitos-de-hardware)
    - [💻 Requisitos de Software](#-requisitos-de-software)
    - [⚙️ Configuração](#️-configuração)
    - [🔧 Compilação e Gravação](#-compilação-e-gravação)
    - [📡 Configuração do Broker MQTT](#-configuração-do-broker-mqtt)
    - [🧪 Testes](#-testes)
  - [🏗️ Estrutura do Código](#️-estrutura-do-código)
  - [💡 Melhorias Futuras](#-melhorias-futuras)
  - [🤝 Como Contribuir (Opcional)](#-como-contribuir-opcional)
  - [📜 Licença (Opcional)](#-licença-opcional)
  - [🎬 Vídeo de Demonstração](#-vídeo-de-demonstração)

---

## 🎯 Objetivos do Projeto

*   Desenvolver um dispositivo IoT robusto na Raspberry Pi Pico W para monitoramento agrícola.
*   Coletar dados ambientais em tempo real: temperatura e umidade do ar (DHT22) e luminosidade ambiente (LDR).
*   Simular o monitoramento do nível de reservatório de água usando um joystick.
*   Implementar comunicação com um broker MQTT para publicação de dados dos sensores e estados dos atuadores.
*   Controlar atuadores (irrigação simulada e iluminação artificial) **localmente através de botões** e reportar seus estados via MQTT.
*   Fornecer feedback visual local usando um display OLED (dados dos sensores, IP, status MQTT) e uma matriz de LEDs (status dos atuadores).
*   Utilizar um LED RGB para indicação do status do sistema (Wi-Fi, conexão MQTT).
*   Consolidar o entendimento da programação C para sistemas embarcados, conectividade Wi-Fi (CYW43439), protocolo MQTT (LwIP) e interação com diversos periféricos (ADC, GPIO, I2C, PIO).

---

## 📝 Descrição do Projeto

Este projeto transforma a Raspberry Pi Pico W em um **"Nó Agrícola Inteligente"**. O sistema se conecta a uma rede Wi-Fi, estabelece uma conexão com um broker MQTT e, em seguida, monitora continuamente as condições ambientais usando sensores reais. Esses dados são publicados periodicamente em tópicos MQTT específicos. Adicionalmente, o sistema permite o **controle local de atuadores simulados (irrigação e iluminação) através de botões físicos no dispositivo**, e o estado desses atuadores é subsequentemente publicado via MQTT.

<!--
    SUGESTÃO: Se possível, adicione um diagrama de blocos/arquitetura aqui.
    Pode ser um .png ou .svg na sua pasta de projeto e referenciado assim:
    ![Diagrama do Sistema](caminho/para/seu/diagrama.png)
-->

### 📊 Fluxo de Dados e Controle

1.  **Coleta de Dados dos Sensores (Reais e Simulados):**
    *   **DHT22 (Sensor Externo):** Coleta leituras reais de **Temperatura do Ar** e **Umidade do Ar**.
    *   **LDR (Sensor Externo):** Mede a **Luminosidade Ambiente** (publicado como "BAIXA", "MEDIA", "ALTA").
    *   **Joystick (ADC):** Simula o **Nível do Reservatório de Água** (publicado como "BAIXO", "MEDIO", "ALTO").
2.  **Controle Local de Atuadores:**
    *   **Botões Físicos:** Permitem ligar/desligar a "Irrigação" e a "Luz Artificial" diretamente no dispositivo.
3.  **Comunicação MQTT:**
    *   **Publicação:**
        *   Dados dos sensores (temperatura, umidade, nível de luminosidade, nível do reservatório) são publicados em tópicos MQTT designados (ex: `pico_w/client_id/sensors/temperature_air`).
        *   O estado atual dos atuadores (Ligado/Desligado), após ser alterado pelos botões locais ou na inicialização, é publicado em tópicos de estado (ex: `pico_w/client_id/actuators/irrigation/state`).
    *   **Inscrição (Subscribe):** *Nesta versão do código, o dispositivo não se inscreve ativamente em tópicos para receber comandos de controle de atuadores via MQTT. O controle é local.* As mensagens MQTT recebidas são logadas no console serial, mas não acionam ações nos atuadores.
4.  **Feedback Local no Pico W:**
    *   **Display OLED (I2C):** Mostra o status da conexão Wi-Fi (endereço IP), status da conexão MQTT e leituras de sensores e estados dos atuadores atualizados periodicamente.
    *   **Matriz de LEDs (WS2812 via PIO):** Representa visualmente o estado dos atuadores:
        *   Ícone de gota d'água quando "Irrigação" está LIGADA.
        *   Ícone de lâmpada quando "Luz Artificial" está LIGADA (se irrigação estiver desligada).
        *   Matriz é limpa quando os atuadores estão DESLIGADOS.
    *   **LED RGB (GPIO/PWM):** Indica o status geral do sistema:
        *   (ex: 🟡 Amarelo: Conectando Wi-Fi)
        *   (ex: 🔵 Ciano: Conectando MQTT)
        *   (ex: 🟢 Verde: Wi-Fi & MQTT Conectados)
        *   (ex: 🔴 Vermelho: Erro - Falha no Wi-Fi ou MQTT)
    *   **LED Integrado do Pico W:** Pisca se o Wi-Fi está conectado, mas o MQTT não; totalmente apagado quando o MQTT está conectado.

### 📡 Comunicação MQTT e Feedback Local

A função `mqtt_connection_cb` lida com o status da conexão MQTT. Após uma conexão bem-sucedida, o estado inicial dos relés (controlados localmente) é publicado. As funções `mqtt_incoming_publish_cb` e `mqtt_incoming_data_cb` atualmente apenas logam no console serial as mensagens recebidas nos tópicos em que o Pico W possa estar inscrito por padrão ou para fins de depuração, sem processá-las para controle de atuadores.

---

## ✨ Funcionalidades Implementadas

✅ Conexão Wi-Fi (Modo Estação) com tratamento de erros.
✅ Implementação de Cliente MQTT (LwIP) para conexão a um broker.
✅ Geração de Client ID único baseado no ID da placa Pico.
✅ Callback de status da conexão MQTT.
✅ Resolução DNS para o hostname do broker MQTT.
✅ **Publicação de Dados dos Sensores via MQTT:**
    ✅ Temperatura do Ar (DHT22) - valor numérico.
    ✅ Umidade do Ar (DHT22) - valor numérico.
    ✅ Luminosidade (LDR) - como "BAIXA", "MEDIA", "ALTA".
    ✅ Nível Simulado do Reservatório (Joystick) - como "BAIXO", "MEDIO", "ALTO".
✅ **Controle Local de Atuadores via Botões:**
    ✅ Controle de Irrigação (LIGA/DESLIGA).
    ✅ Controle de Luz Artificial (LIGA/DESLIGA).
✅ Publicação dos Estados dos Atuadores via MQTT (loop de feedback após controle local).
✅ **Leitura de Sensores em Tempo Real:**
    ✅ DHT22 para temperatura e umidade.
    ✅ LDR para luminosidade.
✅ **Simulação Interativa:**
    ✅ Joystick para entrada do nível do reservatório.
    ✅ Botões para controle dos atuadores.
✅ **Feedback Visual Local:**
    ✅ Display OLED: IP Wi-Fi, status MQTT, valores dos sensores, estados dos atuadores.
    ✅ Matriz de LEDs: Ícones para atuadores ativos (gota d'água, lâmpada).
    ✅ LED RGB: Status do sistema (conectividade Wi-Fi, MQTT, erros).
    ✅ LED Integrado: Indicação de tentativa de conexão MQTT.
✅ Código Estruturado: Organização com arquivos de cabeçalho e implementação para periféricos.
✅ Logging de Depuração: Saída serial (`printf`) para status e depuração.
✅ Configuração Centralizada: Credenciais Wi-Fi (`credentials.h`), configurações MQTT (`mqtt_config.h`), pinos (`include/config.h`).

---

## 🛠️ Requisitos Técnicos Atendidos

*(Esta seção normalmente se alinha com critérios específicos de um curso ou projeto. Adapte conforme necessário.)*

1.  **Funcionalidade Principal:** O sistema conecta-se com sucesso ao Wi-Fi e a um broker MQTT, publica dados dos sensores e os estados dos atuadores. Atuadores são controlados localmente.
2.  **Integração de Periféricos:** Demonstra uso e integração eficazes de DHT22, LDR, Joystick (ADC), Botões (com debouncer), OLED (I2C), Matriz de LEDs (PIO) e LED RGB.
3.  **Comunicação de Rede:** Implementa conectividade Wi-Fi robusta e operações de cliente MQTT (conectar, publicar, callbacks básicos) usando LwIP.
4.  **Qualidade do Código:** Código organizado em múltiplos arquivos (`.c` e `.h`), com nomes claros e comentários que auxiliam no entendimento.
5.  **Princípios de Sistemas Embarcados:** Utiliza operações temporizadas não bloqueantes (verificações de tempo), callbacks para eventos assíncronos (MQTT, DNS) e gerencia o estado do dispositivo.

---

## 🚀 Como Executar

### 🔩 Requisitos de Hardware

*   Raspberry Pi Pico W.
*   Cabo Micro-USB para alimentação e programação.
*   **Sensores e Entradas:**
    *   Sensor de temperatura e umidade DHT22.
    *   LDR (Resistor Dependente de Luz) com um resistor pull-down apropriado (ex: 10kΩ).
    *   Joystick Analógico.
    *   Botões físicos (para controle local dos atuadores).
*   **Periféricos de Saída:**
    *   Display OLED SSD1306 (I2C).
    *   Matriz de LEDs WS2812.
    *   LED RGB de Cátodo Comum ou Anodo Comum (e resistores limitadores de corrente).
    *   (Opcional) Buzzer.
*   Protoboard e fios jumpers.
*   Acesso a uma rede Wi-Fi (2.4 GHz).
*   Um Broker MQTT (ex: Mosquitto instalado localmente, HiveMQ Cloud, Adafruit IO).

### 💻 Requisitos de Software

*   **Pico SDK** (ex: v1.5.1 ou compatível).
*   **CMake** (versão 3.13 ou superior).
*   **ARM GCC Toolchain** (ex: `arm-none-eabi-gcc`).
*   **VS Code** com as extensões "CMake Tools" e "Cortex-Debug" (ou "Pico-W-Go") (recomendado).
*   Um **Terminal Serial** (ex: Minicom, PuTTY, terminal integrado do VS Code) para visualizar a saída do `printf`.
*   Uma **Ferramenta Cliente MQTT** (ex: MQTT Explorer, `mosquitto_pub`/`mosquitto_sub`) para observar publicações.

### ⚙️ Configuração

1.  **Clonar o Repositório (se aplicável):**
    ```bash
    git clone https://github.com/seu-usuario/seu-repositorio.git
    cd seu-repositorio
    ```

2.  **Credenciais Wi-Fi:**
    *   Crie/Edite o arquivo `credentials.h` na raiz do projeto:
    ```c
    // Dentro de credentials.h
    #ifndef CREDENTIALS_H
    #define CREDENTIALS_H

    #define WIFI_SSID_CREDENTIALS "SUA_REDE_WIFI"
    #define WIFI_PASSWORD_CREDENTIALS "SUA_SENHA_WIFI"

    #endif // CREDENTIALS_H
    ```

3.  **Detalhes do Broker MQTT:**
    *   Edite o arquivo `mqtt_config.h` na raiz do projeto:
    ```c
    // Dentro de mqtt_config.h
    #define MQTT_SERVER "endereco_ou_ip_do_seu_broker_mqtt" // ex: "test.mosquitto.org" ou IP
    #define MQTT_PORT 1883 // Ou a porta do seu broker (1883 para não criptografado, 8883 para SSL)
    #define MQTT_USERNAME "seu_usuario_mqtt" // Deixe "" se não houver autenticação
    #define MQTT_PASSWORD "sua_senha_mqtt"   // Deixe "" se não houver autenticação
    #define MQTT_CLIENT_ID_PREFIX "PicoW_AgroNode" // Prefixo para o ID de cliente único

    // Tópicos MQTT completos (o ID da placa será concatenado pelo código se necessário,
    // mas o main.c atual parece usar esses nomes diretamente)
    // Verifique como seu código constrói os nomes finais dos tópicos.
    #define MQTT_TOPIC_TEMP_AR "pico_w/client_id_placeholder/sensors/temperature_air" // Substitua ou ajuste no código
    #define MQTT_TOPIC_UMID_AR "pico_w/client_id_placeholder/sensors/humidity_air"
    #define MQTT_TOPIC_LUMINOSIDADE "pico_w/client_id_placeholder/sensors/luminosity"
    #define MQTT_TOPIC_RESERVATORIO "pico_w/client_id_placeholder/sensors/reservoir_level"
    #define MQTT_TOPIC_IRRIGACAO_SET "pico_w/client_id_placeholder/actuators/irrigation/set" // Não usado para receber comandos neste main.c
    #define MQTT_TOPIC_IRRIGACAO_STATE "pico_w/client_id_placeholder/actuators/irrigation/state"
    #define MQTT_TOPIC_LUZ_SET "pico_w/client_id_placeholder/actuators/light/set" // Não usado para receber comandos neste main.c
    #define MQTT_TOPIC_LUZ_STATE "pico_w/client_id_placeholder/actuators/light/state"

    // Definições de QoS e Retain
    #define MQTT_QOS 1
    #define MQTT_RETAIN_SENSOR_DATA 0
    #define MQTT_RETAIN_STATE_DATA 1 // Estados dos relés são retidos
    ```
    *   **Nota Importante sobre Tópicos:** O `main.c` que você forneceu usa os `MQTT_TOPIC_...` definidos diretamente. Para que o `client_id` único seja parte do tópico, você precisará modificar o `main.c` para construir os nomes completos dos tópicos (ex: `snprintf(full_topic, sizeof(full_topic), "%s/%s/%s", MQTT_CLIENT_ID_PREFIX, board_id_str, "sensors/temperature_air");`) ou ajustar os `#define` acima para incluir um placeholder que você substitua, ou usar uma função que gere o tópico completo. O exemplo acima assume que você vai ajustar os defines ou o código para incluir o ID único.

4.  **Definições de Pinos de Hardware:**
    *   Verifique e ajuste as definições de pinos em `include/config.h` para corresponder à sua fiação física para DHT22, LDR, Joystick, Botões, OLED (SDA/SCL), Matriz de LEDs e LED RGB.

### 🔧 Compilação e Gravação

1.  **Configurar Diretório de Build:**
    ```bash
    mkdir build
    cd build
    ```
2.  **Executar CMake e Make:**
    ```bash
    # Certifique-se de que PICO_SDK_PATH está definido no seu ambiente ou passe-o para o CMake:
    # export PICO_SDK_PATH=/caminho/para/seu/pico-sdk
    cmake ..
    make -j$(nproc) # Ou simplesmente 'make'
    ```
3.  **Gravar no Pico W:**
    *   Segure o botão **BOOTSEL** do seu Pico W enquanto o conecta via USB (ou enquanto o reinicia).
    *   Ele aparecerá como um dispositivo de armazenamento em massa chamado `RPI-RP2`.
    *   Arraste e solte o arquivo `.uf2` gerado (ex: `MQTT-CONECTION-RP2040-W.uf2` ou similar, que estará no diretório `build`) nesta unidade.
    *   O Pico W reiniciará automaticamente e executará o novo firmware.

### 📡 Configuração do Broker MQTT

*   Certifique-se de que seu broker MQTT esteja em execução e acessível pela mesma rede que o Pico W.
*   Se estiver usando autenticação, verifique se o nome de usuário e a senha em `mqtt_config.h` estão corretos e configurados no broker.

### 🧪 Testes

1.  **Monitor Serial:**
    *   Abra um terminal serial conectado à porta COM do Pico W (baud rate geralmente `115200`).
    *   Observe os logs para conexão Wi-Fi, atribuição de endereço IP, tentativas de conexão MQTT e status.
2.  **Ferramenta Cliente MQTT (ex: MQTT Explorer):**
    *   Conecte-se ao seu broker MQTT.
    *   **Inscreva-se (Subscribe)** nos tópicos que seu Pico W está publicando. Exemplo para ver todos os dados de sensores e estados de um dispositivo específico (ajuste `client_id_placeholder` para o ID real da sua placa se você modificar o código para incluí-lo no tópico):
        `pico_w/client_id_placeholder/sensors/#`
        `pico_w/client_id_placeholder/actuators/+/state`
        Você deverá ver os dados dos sensores e estados dos atuadores aparecendo.
    *   **Controle Local:** Use os botões físicos no seu dispositivo Pico W para ligar/desligar a irrigação e a luz.
    *   Observe o display OLED e a matriz de LEDs no Pico W reagindo a esses comandos locais.
    *   Verifique se o Pico W publica o novo estado nos tópicos `/state` correspondentes no seu cliente MQTT.
3.  **Interagir com os Sensores:**
    *   Cubra/descubra o LDR.
    *   Sopre no DHT22 (com cuidado para não umedecer diretamente).
    *   Mova o joystick.
    *   Observe as mudanças refletidas no monitor serial, display OLED e nas publicações MQTT.

---

## 🏗️ Estrutura do Código


.
├── build/                  # Diretório de compilação (gerado)
├── include/
│   ├── external/
│   │   ├── dht22.c
│   │   ├── dht22.h
│   │   ├── ldr.c
│   │   └── ldr.h
│   ├── lib/
│   │   └── ssd1306/
│   │       ├── font.h
│   │       ├── ssd1306.c
│   │       └── ssd1306.h
│   ├── pio/
│   │   └── led_matrix.pio
│   ├── buttons.c
│   ├── buttons.h
│   ├── buzzer.c
│   ├── buzzer.h
│   ├── config.h            # Definições de pinos de hardware, configs globais
│   ├── debouncer.c
│   ├── debouncer.h
│   ├── display.c
│   ├── display.h
│   ├── joystick.c
│   ├── joystick.h
│   ├── led_matrix.c
│   ├── led_matrix.h
│   ├── rgb_led.c
│   └── rgb_led.h
├── CMakeLists.txt          # Script de build do CMake
├── credentials.h           # Credenciais Wi-Fi
├── lwipopts.h              # Configuração da pilha LwIP (se customizado)
├── main.c                  # Lógica principal da aplicação, Wi-Fi, config MQTT, loop principal
├── mbedtls_config.h        # Configuração mbedTLS (para HTTPS/MQTTS futuro)
├── mqtt_config.h           # Broker MQTT, tópicos, credenciais MQTT
├── pico_sdk_import.cmake   # Importação padrão do Pico SDK
├── .gitignore
└── README.md
