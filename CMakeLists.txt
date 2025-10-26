# CMAKE setup
cmake_minimum_required(VERSION 4.0)
project(model-matrix)
set(CMAKE_CXX_STANDARD 20)

# Get IMGUI
set(IMGUI_VERSION 1.91.8)
FetchContent_MakeAvailable(
        imgui
        URL https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}-docking.tar.gz
)


add_executable(model-matrix main.cpp)
