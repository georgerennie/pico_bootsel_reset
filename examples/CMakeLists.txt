cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
project(examples)
pico_sdk_init()

add_subdirectory(../ ${CMAKE_CURRENT_BINARY_DIR}/bootsel_reset)

add_executable(blinky_bootsel_reset
	blinky_bootsel_reset.c
)

target_link_libraries(blinky_bootsel_reset
	pico_stdlib
	bootsel_reset
)

pico_add_extra_outputs(blinky_bootsel_reset)
