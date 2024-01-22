cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rp2040-pio-restart)

set(CMAKE_C_STANDARD 11)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(rp2040-pio-restart
	rp2040_pio_restart.c
)

pico_generate_pio_header(rp2040-pio-restart ${CMAKE_CURRENT_LIST_DIR}/pio1.pio)
pico_generate_pio_header(rp2040-pio-restart ${CMAKE_CURRENT_LIST_DIR}/pio2.pio)

target_link_libraries(rp2040-pio-restart pico_stdlib hardware_pio hardware_dma)

pico_enable_stdio_usb(rp2040-pio-restart 1) 
pico_enable_stdio_uart(rp2040-pio-restart 0) 

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(rp2040-pio-restart)

