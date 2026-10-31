cmake_minimum_required(VERSION 3.16)
project(quant_advisor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")  # CMake's default would be -O3
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(CURL REQUIRED)

# Everything except main.cpp goes in a library, so the tests can link the same code.
file(GLOB QUANT_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/demo/*.cpp)
list(FILTER QUANT_SOURCES EXCLUDE REGEX "src/main\\.cpp$")

add_library(quant_core STATIC ${QUANT_SOURCES})
target_include_directories(quant_core PUBLIC src)
target_include_directories(quant_core SYSTEM PRIVATE third_party)
target_link_libraries(quant_core PUBLIC CURL::libcurl)

add_executable(quant_advisor src/main.cpp)
target_link_libraries(quant_advisor PRIVATE quant_core)

add_executable(quant_tests tests/test_all.cpp)
target_include_directories(quant_tests PRIVATE tests)
target_link_libraries(quant_tests PRIVATE quant_core)

enable_testing()
add_test(NAME quant_tests COMMAND quant_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Copy the UCRT64 DLLs next to the exes so they also run from plain cmd.exe and by double-click.
if(WIN32)
  get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
  foreach(exe quant_advisor quant_tests)
    add_custom_command(TARGET ${exe} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${exe}> -DDEST=$<TARGET_FILE_DIR:${exe}>
              -DSEARCH=${MINGW_BIN_DIR} -DOBJDUMP=${CMAKE_OBJDUMP}
              -P ${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake
      VERBATIM)
  endforeach()
endif()
