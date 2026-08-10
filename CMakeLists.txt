# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Nan0berryOS C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Executavel do Nan0berryOS apontando para a raiz e para o kernel
add_executable(Nan0berryOS 
    src/Nan0berryOS.c
    src/nanoberry.c
    src/kernel/port_asm.S
)

pico_set_program_name(Nan0berryOS "Nan0berryOS")
pico_set_program_version(Nan0berryOS "1.0.0-STABLE")

# Ativa a comunicacao Serial via USB
pico_enable_stdio_uart(Nan0berryOS 0)
pico_enable_stdio_usb(Nan0berryOS 1)

# Diretorio das pastas de cabecalho (.h)
target_include_directories(Nan0berryOS PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

# Bibliotecas do SDK para o Nan0berry-Kernel
target_link_libraries(Nan0berryOS 
        pico_stdlib
        hardware_timer
        hardware_irq
        hardware_sync
        pico_multicore
)

pico_add_extra_outputs(Nan0berryOS)