cmake_minimum_required(VERSION 3.0.0)
project(runtimestruct)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog CONFIG REQUIRED)

include_directories(include)

include_directories(deps/tomlplusplus/include)

add_executable(runtimestruct src/main.cpp)
target_link_libraries(runtimestruct PRIVATE spdlog::spdlog)