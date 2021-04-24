# Exchange the current test target in add_executable

add_executable(target_test_saphBme280 test_saphBme280.c)

target_include_directories(target_test_saphBme280 PUBLIC ../unity/ ../src/ ../build/test/mocks)
target_link_directories(target_test_saphBme280 PRIVATE ../unity/ ../src/ ../build/test/mocks)

# Exchange the current test CUT in target_link_libraries
target_link_libraries(target_test_saphBme280 unity_lib pico_stdlib)


add_executable(target_test_saphBme280_internal test_saphBme280_internal.c)

target_include_directories(target_test_saphBme280_internal PUBLIC ../unity/ ../src/ ../build/test/mocks)
target_link_directories(target_test_saphBme280_internal PRIVATE ../unity/ ../src/ ../build/test/mocks)

# Exchange the current test CUT in target_link_libraries
target_link_libraries(target_test_saphBme280_internal unity_lib pico_stdlib)
