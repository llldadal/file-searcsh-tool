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

add_executable(
    file_search_tests
    text/v0.2_text.cpp
)

target_link_libraries(
    file_search_tests
    PRIVATE
    search_core
)