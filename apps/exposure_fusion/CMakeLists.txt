cmake_minimum_required(VERSION 3.16)
project(exposure)

enable_testing()

# Set up language settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Find Halide
find_package(Halide REQUIRED)

# Generator
add_executable(exposure.generator exposure_generator.cpp)
target_link_libraries(exposure.generator PRIVATE Halide::Generator Halide::Tools)

# Processs
add_halide_library(exposure FROM exposure.generator)
add_halide_library(exposure_auto_schedule FROM exposure.generator
                   GENERATOR exposure
                   PARAMS auto_schedule=true)

# Main executable
add_executable(exposure_process process.cpp)
target_link_libraries(exposure_process
                      PRIVATE
                      Halide::ImageIO
                      exposure
                      exposure_auto_schedule)

# Test that the app actually works!
set(IMAGE ${CMAKE_CURRENT_LIST_DIR}/../images/rgba.png)
if (EXISTS ${IMAGE})
    configure_file(${IMAGE} rgba.png COPYONLY)
    add_test(NAME exposure_process COMMAND exposure_process rgba.png out.png)
    set_tests_properties(exposure_process PROPERTIES
                         LABELS internal_app_tests
                         PASS_REGULAR_EXPRESSION "Success!"
                         SKIP_REGULAR_EXPRESSION "\\[SKIP\\]")
endif ()
