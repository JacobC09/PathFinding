cmake_minimum_required(VERSION 3.25)
project(ChristmasGame)

option(BUILD_EXAMPLES "" OFF)

add_subdirectory(external/raylib)
add_subdirectory(external/rlImGui)

add_executable(game 
    src/main.cpp
    src/grid.cpp
)

target_precompile_headers(game PUBLIC src/include/pch.h)
target_link_libraries(game PRIVATE raylib rlImGui)
target_include_directories(game PRIVATE src/include)
add_compile_options(-Wall -std=c++17)

# CMAKE_BUILD_TYPE(RELEASE)
# add_custom_target(copy_assets
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
# )

# add_dependencies(game copy_assets)

if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++17 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY --shell-file C:/raylib/raylib/src/customshell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# emcmake cmake .. -DPLATFORM=Web && cmake --build .
# --preload-file assets