# Use INTERFACE as reccommended by Pico SDK devs
add_library(icsp INTERFACE)

pico_generate_pio_header(icsp ${CMAKE_CURRENT_LIST_DIR}/icsp.pio)

target_sources(icsp INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/icsp.c
  ${CMAKE_CURRENT_LIST_DIR}/icsp.h
)

target_include_directories(icsp INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(icsp INTERFACE pico_stdlib hardware_pio)
