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

# Store commonly used paths in vars
set(ICON_FILE "AppIcon.icns")
set(ICON_PATH "./res/icon/${ICON_FILE}")
set(APP_FONTS
        "./res/fonts/Inter-Regular.ttf"
        "./res/fonts/JetBrainsMono-Regular.ttf"
)

# Create executable with all sources and headers
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${ICON_PATH} ${APP_FONTS})

# build imgui library
add_library(ImGui STATIC
        ./external/imgui-1.92.1-docking/imgui.cpp
        ./external/imgui-1.92.1-docking/imgui_draw.cpp
        ./external/imgui-1.92.1-docking/imgui_demo.cpp
        ./external/imgui-1.92.1-docking/imgui_widgets.cpp
        ./external/imgui-1.92.1-docking/imgui_tables.cpp
        ./external/rlImGui/rlImGui.cpp
)

# Build MacOS app bundle if applicable
if (APPLE)
    message("Building macOS app bundle")
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_ICON_FILE ${ICON_FILE}
    )

    message("Adding icon to bundle resource")
    set_source_files_properties(${ICON_PATH} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )

    message("Adding fonts to bundle resource")
    set_source_files_properties(${APP_FONTS} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )

    # Patch the dylibs into the bundle
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUILD_RPATH "@executable_path/../Frameworks"
            INSTALL_RPATH "@executable_path/../Frameworks"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks"

            # COPY EACH DYLIB INTO THE BUNDLE
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/external/raylib-5.5_macos/lib/libraylib.5.5.0.dylib"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/libraylib.5.5.0.dylib"

            # Recreate symlinks
            COMMAND ln -sf
            "libraylib.5.5.0.dylib"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/libraylib.550.dylib"

            COMMAND ln -sf
            "libraylib.550.dylib"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/libraylib.dylib"

            # Fix install name of the real dylib
            COMMAND install_name_tool
            -id "@rpath/libraylib.dylib"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/libraylib.5.5.0.dylib"
    )

endif()

# Handle linking for different platforms
if(WIN32)
    message("WIN32 detected! Linking Winmm")
    target_link_libraries(${PROJECT_NAME} PRIVATE
            raylib
            ImGui
            Winmm
    )
elseif(EMSCRIPTEN)
    message("Passing WASM-specific linkage flags")
    target_link_options(
            ${PROJECT_NAME}
            PRIVATE
            -sALLOW_MEMORY_GROWTH=1
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            raylib
            ImGui
            "-framework CoreFoundation"
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
            raylib
            ImGui
    )
endif()