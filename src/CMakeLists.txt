# You need a lib target for each static software lib.
# This helps you later to specifically include it as CUTs
# Replace/extend these as you see fit. 

add_subdirectory(integration_tests)

# TODO: Learn how to statically add headers to link later against

add_library(saphBme280 STATIC
        saphBme280.c
        )

target_link_libraries(saphBme280
        pico_stdlib
        pico_time
        hardware_i2c
        hardware_gpio
        )


add_library(saphBme280_internal STATIC
        saphBme280_internal.c
        )

target_link_libraries(saphBme280_internal
        pico_stdlib
        pico_time
        hardware_i2c
        hardware_gpio
        )


# I2C helper lib to make the rest testable.
add_library(i2c_handler STATIC
        i2c_handler.c)

target_link_libraries(i2c_handler
        pico_stdlib
        hardware_i2c
        )