add_executable(TestSuite)

set(IncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/include/")
set(ROOT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(IncludeFiles "Utils.h")
list(TRANSFORM IncludeFiles PREPEND ${IncludeDir})

add_subdirectory(demo)

target_sources(
        TestSuite
        PRIVATE
        FILE_SET HEADERS
        BASE_DIRS ${IncludeDir}
        FILES ${IncludeFiles}
)

target_include_directories(
        TestSuite PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
        TestSuite PRIVATE
        GTest::gtest_main
        StrictWarnings
        DemoTests
)

include(GoogleTest)
gtest_discover_tests(TestSuite)