add_executable(storage_test
    storage_test.cpp
)

target_include_directories(storage_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/include
)

target_link_libraries(storage_test
    PRIVATE
    ticket_system_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(storage_test)
