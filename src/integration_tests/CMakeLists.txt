# You need a lib target for each static software lib.
# This helps you later to specifically include it as CUTs
# Replace/extend these as you see fit. 


add_executable(integration_saphBme280
        integration_saphBme280.c
)

target_link_libraries(integration_saphBme280
        saphBme280
        i2c_handler

        # Libraries provided by the pico sdk
        pico_stdlib
        pico_time
        hardware_i2c
        hardware_gpio
        )

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(integration_saphBme280)

pico_enable_stdio_usb(integration_saphBme280 1)
pico_enable_stdio_uart(integration_saphBme280 0)
