cmake_minimum_required(VERSION 3.25)
project(GenericProject LANGUAGES CXX)
enable_testing()


# Turn on solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(DevToolsDir "Dev Tools")
include(cmake/ClangFormat.cmake)
include(cmake/ClangTidy.cmake)
include(cmake/Doxygen.cmake)
include(cmake/StrictMode.cmake)
include(cmake/Asan.cmake)
include(cmake/Dependencies.cmake)

# Multi-processor compilation
if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Enabling Multiprocessor compilation on MSVC")
    add_compile_options("/MP")
endif ()

# Interprocedural Optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORT)
if(IPO_SUPPORTED)
    message(STATUS "Turning on Interprocedural Optimization")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${IPO_SUPPORTED})
endif ()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestSuite)