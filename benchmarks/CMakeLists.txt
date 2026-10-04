add_executable(BenchmarkSuite)

target_sources(
        BenchmarkSuite
        PRIVATE
        SampleBenchmark.cpp
)

target_precompile_headers(
        BenchmarkSuite
        PRIVATE
        <benchmark/benchmark.h>
)

target_include_directories(BenchmarkSuite PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
        BenchmarkSuite
        PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
        StrictWarnings
)