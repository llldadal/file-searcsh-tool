cmake_minimum_required(VERSION 3.20)

project(
    file-search-engine
    VERSION 0.3
    LANGUAGES CXX
)

add_library(
    search_core
    src/fileinfo.cpp
    src/search.cpp
)

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
    file_search_text
    text/v0.2_text.cpp
)

target_link_libraries(
    file_search_text
    PRIVATE
    search_core
)