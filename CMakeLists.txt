cmake_minimum_required(VERSION 3.10)
project(Ticket-System-2025)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a header-only library for template classes
add_library(ticket_system_lib INTERFACE)

include_directories(ticket_system_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Google Test
# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
#     DOWNLOAD_EXTRACT_TIMESTAMP TRUE
# )
# FetchContent_MakeAvailable(googletest)

# enable_testing()

# # Add test directory
# add_subdirectory(test)

# Clean up data files command
add_custom_target(clean_data_files
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/data_data ${CMAKE_BINARY_DIR}/data_node
    COMMENT "Removing data_data and data_node files"
)

add_executable(code ${CMAKE_CURRENT_SOURCE_DIR}/src/submit/BPlusTree.cpp)
add_dependencies(code clean_data_files)