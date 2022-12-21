cmake_minimum_required(VERSION 3.25)
project(rlImGui)

file(GLOB rlImGui_SRC
     "*.h"
     "*.cpp"
)

add_library(rlImGui STATIC ${rlImGui_SRC})
target_link_libraries(rlImGui PRIVATE raylib)
target_include_directories(rlImGui PUBLIC .)
