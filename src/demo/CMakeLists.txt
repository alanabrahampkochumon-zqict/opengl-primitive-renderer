add_executable(DemoApp)
target_compile_features(DemoApp PRIVATE cxx_std_20)


target_sources(
        DemoApp
        PRIVATE
            Main.cpp

        PRIVATE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            Utils.h
)

target_include_directories(
        DemoApp PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
        DemoApp PRIVATE
        StrictWarnings
)
