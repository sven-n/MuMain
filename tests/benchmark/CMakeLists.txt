mu_add_test(
    NAME test_bench_stats
    SOURCES
        test_bench_stats.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchStats.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchTypes.cpp
)

mu_add_test(
    NAME test_bench_segment
    SOURCES
        test_bench_segment.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchSegment.cpp
)

mu_add_test(
    NAME test_bench_report
    SOURCES
        test_bench_report.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchFindings.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchReport.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchStats.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Benchmark/BenchTypes.cpp
)
