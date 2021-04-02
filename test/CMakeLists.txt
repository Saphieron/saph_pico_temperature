# Exchange the current test target in add_executable

add_executable(test_target test_saphBme280.c)

target_include_directories(test_target PUBLIC ../unity/ ../src/ ../build/test/mocks)
target_link_directories(test_target PRIVATE ../unity/ ../src/ ../build/test/mocks)

# Exchange the current test CUT in target_link_libraries
target_link_libraries(test_target unity_lib pico_stdlib)
