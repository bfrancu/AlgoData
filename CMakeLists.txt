cmake_minimum_required(VERSION 3.18)

project(algo_data_structures VERSION 1.0 LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(data_structures INTERFACE)
target_include_directories(data_structures 
         INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/data>
         INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

target_compile_options(data_structures INTERFACE -std=c++17)

add_library(algo INTERFACE)
target_include_directories(algo 
         INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/algo>
         INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)
target_compile_options(algo INTERFACE -std=c++17)

OPTION(PROFILE_MAIN "Add gprof compile flags" OFF)
# profile probes will slow down execution
# disable PGO_MAIN_GENERATE after the profiling data is obtained to avoid probes slow down
option(PGO_MAIN_GENERATE "Generate profile guided compiler optimization" OFF)
OPTION(PGO_MAIN_USE "Use profile guided compiler optimization" OFF)
OPTION(PRINT_OPTIMIZATION_REPORT "Print gcc optimization report" ON)

add_executable(main ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)
target_link_libraries(main PRIVATE data_structures algo)

target_compile_options(main PRIVATE -g -fno-omit-frame-pointer)

if(PROFILE_MAIN)
    target_compile_options(main PRIVATE -pg)
    target_link_options(main PRIVATE -pg)
endif()

if(PGO_MAIN_GENERATE)
    target_compile_options(main PRIVATE -fprofile-generate)
    target_link_options(main PRIVATE -fprofile-generate)
endif()

if(PGO_MAIN_USE)
    target_compile_options(main PRIVATE -fprofile-use)
    target_link_options(main PRIVATE -fprofile-use)
endif()

if(PRINT_OPTIMIZATION_REPORT)
    add_executable(opt_report ${CMAKE_CURRENT_LIST_DIR}/src/opt_info_report.cpp)
    target_link_libraries(opt_report PRIVATE data_structures)
    target_compile_options(opt_report PRIVATE -O3 -march=core-avx2 -fopt-info)
endif()

include(CMakePrintHelpers)
cmake_print_properties(TARGETS main PROPERTIES 
                                    TYPE 
                                    COMPILE_DEFINITIONS 
                                    COMPILE_OPTIONS
                                    LINK_OPTIONS)

include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929
)

FetchContent_GetProperties(googletest)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()


add_executable(data_structures_test ${CMAKE_CURRENT_LIST_DIR}/test/test_main.cpp)
target_link_libraries(data_structures_test PRIVATE gtest_main PRIVATE data_structures)
target_include_directories(data_structures_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/test)
target_compile_options(data_structures_test PRIVATE -g -fno-omit-frame-pointer)


include(GoogleTest)
gtest_discover_tests(data_structures_test)


FetchContent_Declare(benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        f91b6b42b1b9854772a90ae9501464a161707d1e
)

FetchContent_GetProperties(benchmark)

if (NOT benchmark_POPULATED)
    FetchContent_Populate(benchmark)
    add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
endif()

if(BENCHMARK_ENABLE_TESTING)
    option(BENCHMARK_ENABLE_TESTING "Disable benchmark testing." OFF)
endif()

add_executable(data_structures_benchmark ${CMAKE_CURRENT_LIST_DIR}/benchmark/benchmark_main.cpp)
target_link_libraries(data_structures_benchmark PRIVATE benchmark::benchmark 
                                                PRIVATE data_structures)

target_include_directories(data_structures_benchmark PRIVATE {CMAKE_CURRENT_LIST_DIR}/benchmark)
target_compile_options(data_structures_benchmark PRIVATE -g -fno-omit-frame-pointer)

option(BENCHMARK_LIST "run benchmarks for ds::list" OFF)
option(BENCHMARK_VECTOR "run benchmarks for ds::vector" ON)

if (BENCHMARK_LIST)
    target_compile_definitions(data_structures_benchmark PRIVATE RUN_LIST_BENCHMARK=1)
endif()

if (BENCHMARK_VECTOR)
    target_compile_definitions(data_structures_benchmark PRIVATE RUN_VECTOR_BENCHMARK=1)
endif()

#TODO
#Make functions to be able to support comparative benchmarks
#Have a distinct set of benchmarks and select them at compile time
#For example benchmark_select(LIST, VECTOR) will add only the list and vector benchmarks
#to the data_strutures_benchmark executable
