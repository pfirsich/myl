cmake_minimum_required(VERSION 3.0.0)
project(myl)

set(CMAKE_CXX_STANDARD 17)

set(SRC
    main.cpp
    componentfile.cpp
    fieldtype.cpp
)
list(TRANSFORM SRC PREPEND src/)

find_package(spdlog CONFIG REQUIRED)

include_directories(include)

include_directories(deps/tomlplusplus/include)

add_executable(myl ${SRC})
target_link_libraries(myl PRIVATE spdlog::spdlog)