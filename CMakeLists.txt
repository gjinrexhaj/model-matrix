# CMAKE setup
cmake_minimum_required(VERSION 4.0)
project(model-matrix)
set(CMAKE_CXX_STANDARD 20)

# Check which platform we're building for, include appropriate directories
if(APPLE)
    message("Configuring CMake for OSX!")
    include_directories(
            "./external/raylib-5.5_macos/include"
    )
    link_directories("./external/raylib-5.5_macos/lib")
elseif(WIN32)
    message("Configuring CMake for Windows!")
    include_directories(
            "./external/raylib-5.5_win64_msvc16/include"
    )
    link_directories("./external/raylib-5.5_win64_msvc16/lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("Configuring CMake for Linux!")
    include_directories(
            "./external/raylib-5.5_linux_amd64/include"
    )
    link_directories("./external/raylib-5.5_linux_amd64/lib")
elseif(EMSCRIPTEN)
    message("Configuring CMake for WASM")
    include_directories(
            "./external/raylib-5.5_webassembly/include"
    )
    link_directories("./external/raylib-5.5_webassembly/lib")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

include_directories(
        "./external/rlImGui"
        "./external/imgui-1.92.1-docking/"
)


# Group all application sources and headers
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)


# Create executable with all sources and headers
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# build imgui library
add_library(ImGui STATIC
        ./external/imgui-1.92.1-docking/imgui.cpp
        ./external/imgui-1.92.1-docking/imgui_draw.cpp
        ./external/imgui-1.92.1-docking/imgui_demo.cpp
        ./external/imgui-1.92.1-docking/imgui_widgets.cpp
        ./external/imgui-1.92.1-docking/imgui_tables.cpp
        ./external/rlImGui/rlImGui.cpp
)

# Link pre-built raylib and build ImGui libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        raylib
        ImGui
)


# Set compile flags for WASM target during program linkage
if(EMSCRIPTEN)
    message("Passing WASM-specific linkage flags")
    target_link_options(
            ${PROJECT_NAME}
            PRIVATE
            -sALLOW_MEMORY_GROWTH=1
    )
endif()