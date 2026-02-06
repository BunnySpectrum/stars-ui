cmake_minimum_required(VERSION 3.20)
project(stars LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- SDL2 (static) ---
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.32.10
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL2)

# --- Dear ImGui ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL2::SDL2-static)
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

# --- ImPlot ---
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v0.16
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(implot)

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# --- SQLite3 ---
FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake.git
    GIT_TAG        v3.49.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(sqlite3)

# --- nanosvg ---
FetchContent_Declare(
    nanosvg
    GIT_REPOSITORY https://github.com/memononen/nanosvg.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nanosvg)
# The upstream CMakeLists only sets INSTALL_INTERFACE; add BUILD_INTERFACE so
# our code can #include "nanosvg.h" during the build.
target_include_directories(nanosvg PUBLIC
    $<BUILD_INTERFACE:${nanosvg_SOURCE_DIR}/src>)

# --- OpenGL ---
find_package(OpenGL REQUIRED)

# --- Main executable ---
add_executable(stars
    src/main.cpp
    src/stars.cpp
    src/field_store.cpp
    src/field_renderer.cpp
    src/db_reader.cpp
    src/svg_renderer.cpp
)
target_link_libraries(stars PRIVATE implot nanosvg OpenGL::GL SQLite::SQLite3)

# macOS: suppress OpenGL deprecation warnings
if(APPLE)
    target_compile_definitions(stars PRIVATE GL_SILENCE_DEPRECATION)
endif()
