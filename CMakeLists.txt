cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME cos)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

project(${PROJECT_NAME})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(INCLUDE_DIRS ${INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIRS})

file(GLOB_RECURSE ASM_SRC_FILES src/*/*.asm)
set_source_files_properties(${ASM_SRC_FILES) PROPERTIES LANGUAGE ASM_NASM)

file(GLOB_RECURSE SRC_FILES src/*/*.c)

add_executable(kernel ${SRC_FILES} ${ASM_SRC_FILES})

