add_library(DemoTests OBJECT)

target_sources(
        DemoTests PRIVATE
        SampleTest.cpp
)

target_include_directories(
        DemoTests PRIVATE
        ${ROOT_TEST_DIR}/include
)

target_link_libraries(
        DemoTests PRIVATE
        GTest::gtest
        GTest::gtest_main
        StrictWarnings
)