cmake_minimum_required(VERSION 3.20)

project(
    file_search_engine
    VERSION 0.3
    LANGUAGES CXX
)

add_library(
    search_core
    STATIC
    src/fileinfo.cpp
    src/search.cpp
    src/interaction.cpp
)

if(MSVC)
    target_compile_options(search_core PUBLIC /utf-8)
endif()

target_compile_features(
    search_core
    PUBLIC
    cxx_std_17
)

target_include_directories(
    search_core
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

add_executable(
    file_search
    main.cpp
)

target_link_libraries(
    file_search
    PRIVATE
    search_core
)

include(CTest)

if(BUILD_TESTING)
    # 测试1
add_executable(
    test_search
    tests/test_support.cpp
    tests/search_tests.cpp
)

target_link_libraries(
    test_search
    PRIVATE
    search_core
)

add_test(
    NAME test_search
    COMMAND test_search
)
# 测试2
add_executable(
    test_parsing
    tests/test_support.cpp
    tests/parsing_tests.cpp
)

target_link_libraries(
    test_parsing
    PRIVATE
    search_core
)

add_test(
    NAME test_parsing
    COMMAND test_parsing
)
# 测试3
add_executable(
    test_interaction
    tests/test_support.cpp
    tests/interaction_tests.cpp
)

target_link_libraries(
    test_interaction
    PRIVATE
    search_core
)

add_test(
    NAME test_interaction
    COMMAND test_interaction
)
endif()