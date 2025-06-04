# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(Mqtt C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${CMAKE_PROJECT_NAME}
    main.c 
    include/lib/ssd1306/ssd1306.c
    include/buzzer.c
    include/debouncer.c
    include/display.c
    include/led_matrix.c
    include/joystick.c
    include/rgb_led.c  
    include/external/dht22.c  
    include/external/ldr.c  
)

pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/include/pio/led_matrix.pio)

target_link_libraries(${CMAKE_PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_adc
    hardware_i2c
    hardware_pwm
    hardware_clocks
    hardware_irq
    hardware_pio
    hardware_timer
    pico_bootrom
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_mbedtls
    pico_lwip_mbedtls
)

# Add the standard include files to the build
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)

pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})