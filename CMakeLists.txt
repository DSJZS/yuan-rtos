cmake_minimum_required(VERSION 3.22)

project(yuan_rtos LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

file(GLOB_RECURSE YUAN_RTOS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.S"
    "${CMAKE_CURRENT_SOURCE_DIR}/libcpu/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/libcpu/*.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/libcpu/*.S"
)

add_library(yuan_rtos STATIC)
add_library(yuan::rtos ALIAS yuan_rtos)

target_sources(yuan_rtos
    PRIVATE
        ${YUAN_RTOS_SOURCES}
)

target_include_directories(yuan_rtos
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libcpu
        ${CMAKE_CURRENT_SOURCE_DIR}/libcpu/CM3
)

target_compile_features(yuan_rtos PUBLIC c_std_11)

set_target_properties(yuan_rtos PROPERTIES
    OUTPUT_NAME "yuan_rtos"
)
