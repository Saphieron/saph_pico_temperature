cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
# note: For CLion projects you will need to add the Environment variable to

include(pico_sdk_import.cmake)

project(saph_pico_temperature)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()


##############
# Testing part
# It also needs to understand we have code in source and test directories
add_subdirectory(src)
add_subdirectory(unity)
add_subdirectory(test)

add_executable(saph_pico_temperature
        src/main.c
        )

# This is the release target' library linking
# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(saph_pico_temperature
        PRIVATE
        # Any libs defined in the src/CMakeLists.txt
        i2c_handler

        # Libraries provided by the pico sdk
        pico_stdlib
        pico_time
        hardware_i2c
        hardware_gpio
        )


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(saph_pico_temperature)

pico_enable_stdio_usb(saph_pico_temperature 1)
pico_enable_stdio_uart(saph_pico_temperature 0)