#SaphBme280_internal tests
add_executable(target_test_saphBme280 test_saphBme280.c)
target_include_directories(target_test_saphBme280 PUBLIC ../unity/ ../src/ ../build/test/mocks .)
target_link_directories(target_test_saphBme280 PRIVATE ../unity/ ../src/ ../build/test/mocks .)
target_link_libraries(target_test_saphBme280 unity_lib pico_stdlib)

#SaphBme280_internal tests
add_executable(target_test_saphBme280_internal test_saphBme280_internal.c)
target_include_directories(target_test_saphBme280_internal PUBLIC ../unity/ ../src/ ../build/test/mocks ./)
target_link_directories(target_test_saphBme280_internal PRIVATE ../unity/ ../src/ ../build/test/mocks ./)
target_link_libraries(target_test_saphBme280_internal unity_lib pico_stdlib)


#saph_ssd1306 tests
add_executable(target_test_saph_ssd1306 test_saph_ssd1306.c)
target_include_directories(target_test_saph_ssd1306 PUBLIC ../unity/ ../src/ ../build/test/mocks ./)
target_link_directories(target_test_saph_ssd1306 PRIVATE ../unity/ ../src/ ../build/test/mocks ./)
target_link_libraries(target_test_saph_ssd1306 unity_lib pico_stdlib)

#saph_ssd1306_internal tests
add_executable(target_test_saph_ssd1306_internal test_saph_ssd1306_internal.c)
target_include_directories(target_test_saph_ssd1306_internal PUBLIC ../unity/ ../src/ ../build/test/mocks ./)
target_link_directories(target_test_saph_ssd1306_internal PRIVATE ../unity/ ../src/ ../build/test/mocks ./)
target_link_libraries(target_test_saph_ssd1306_internal unity_lib pico_stdlib)
