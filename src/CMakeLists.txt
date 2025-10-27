# This is the main binary. List your C files here.
add_executable(can
        can.c
    )

pico_set_program_name(act "test")
pico_set_program_version(act "0.1")

# Modify the below lines to enable/disable output over UART/USB.
# These are mutually exclusive, only set one.
pico_enable_stdio_uart(act 1)
pico_enable_stdio_usb(act 0)

# Add the libraries to the build
target_link_libraries(act PRIVATE
  pico_stdlib
  pico_multicore
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
)

# The CYW43 is the Wifi/Bluetooth module. If the board is set to pico_w, this
# variable will be set and the wireless libraries added.
if(PICO_CYW43_SUPPORTED)
    # For led support on pico_w
    target_link_libraries(act PRIVATE
        pico_cyw43_arch_none
    )
endif()

# Another pico macro which generates the uf2 file in addition to the elf
pico_add_extra_outputs(act)

add_custom_target(flash picotool load -f $<TARGET_FILE:act> DEPENDS act)
