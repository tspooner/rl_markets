cmake_minimum_required(VERSION 3.2.2)
project(rl_markets VERSION 0.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Compiler options
set(GLOBAL_FLAGS "-std=c++1y -Wall -Wextra -FPIC -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLOBAL_FLAGS}")

set(DEBUG_FLAGS "-O0 -g -D DEBUG")
set(RELEASE_FLAGS "-O4 -D NDEBUG")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#####################################################################
#####################################################################

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCOV_LINK_FLAGS}")

set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Load external requirements
add_subdirectory(ext)

# Compile program
include_directories(include
    ${YAML_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})
add_subdirectory(src)

# Unit testing
enable_testing(TRUE)
add_subdirectory(test)

# Add config files:
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
file(GLOB CONFIG_FILES ${CMAKE_SOURCE_DIR}/config/*.yaml)
foreach(cf ${CONFIG_FILES})
    configure_file(${cf} ${CMAKE_BINARY_DIR}/config COPYONLY)
endforeach()

# Add the output directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Generate syntastic completion config file:
create_syntastic_config()
