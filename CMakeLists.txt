cmake_minimum_required(VERSION 3.10)
project(Ticket-System-2025)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Create a header-only library for template classes
add_library(ticket_system_lib INTERFACE)

include_directories(ticket_system_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


find_package(Crow)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# # Google Test
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
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/*_data ${CMAKE_BINARY_DIR}/*_node
    COMMENT "Removing files ending with _data and _node"
)

# add_executable(code ${CMAKE_CURRENT_SOURCE_DIR}/src/submit/BPlusTree.cpp)
# add_dependencies(code clean_data_files)

add_executable(code ${CMAKE_CURRENT_SOURCE_DIR}/src/submit/TicketSystem.cpp)
add_dependencies(code clean_data_files)

add_executable(ticket_http ${CMAKE_CURRENT_SOURCE_DIR}/src/http/HttpServer.cpp )
target_include_directories(ticket_http PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
target_link_libraries(ticket_http PUBLIC Crow::Crow)
target_link_libraries(ticket_http PRIVATE ticket_system_lib OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)
target_compile_features(ticket_http PRIVATE cxx_std_17)
add_dependencies(ticket_http clean_data_files)