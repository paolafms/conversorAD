# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o Pico SDK
include(pico_sdk_import.cmake)

project(conversorAD C CXX ASM)

# Inicializa o Pico SDK
pico_sdk_init()

# Adiciona o diretório "lib" ao caminho de inclusão
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib)

# Adiciona o executável principal
add_executable(conversorAD
    conversorAD.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306.c
    ${CMAKE_CURRENT_LIST_DIR}/lib/font.h
)

pico_set_program_name(conversorAD "conversorAD")
pico_set_program_version(conversorAD "0.1")

pico_enable_stdio_uart(conversorAD 0)
pico_enable_stdio_usb(conversorAD 1)

# Linka as bibliotecas do Pico SDK necessárias
target_link_libraries(conversorAD
        pico_stdlib
        hardware_adc    # Para "hardware/adc.h"
        hardware_pwm    # Para controle PWM
        hardware_i2c    # Para comunicação I2C com o display
)

target_include_directories(conversorAD PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(conversorAD)
include_directories(${PICO_SDK_PATH}/src/rp2_common/hardware_pwm/include)