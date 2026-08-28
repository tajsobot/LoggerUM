cmake_minimum_required(VERSION 4.2)
project(LoggerUM)

set(CMAKE_CXX_STANDARD 20)

# Route everything CMake builds (your exe, SDL3's own dll) into the same
# output folder, so SDL3.dll ends up next to your exe with no manual copy.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

include(FetchContent)

# ---- GoIO SDK (sensor communication) ----
set(GOIO_SDK_DIR ${CMAKE_SOURCE_DIR}/libraries/GoIO/redist)

# ---- SDL3 (windowing/input/GL context) -- vendored, not fetched ----
add_subdirectory(libraries/SDL EXCLUDE_FROM_ALL)

# ---- Dear ImGui (GUI widgets) ----
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.0   # check github.com/ocornut/imgui/releases for anything newer
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ---- One target -- everything lives in main.cpp from here on ----
add_executable(LoggerUM main.cpp ${IMGUI_SOURCES})

target_include_directories(LoggerUM PRIVATE
        ${GOIO_SDK_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

if (WIN32)
    target_compile_definitions(LoggerUM PRIVATE TARGET_OS_WIN)
elseif (APPLE)
    target_compile_definitions(LoggerUM PRIVATE TARGET_OS_MAC)
elseif (LINUX)
    target_compile_definitions(LoggerUM PRIVATE TARGET_OS_LINUX)
endif()

target_link_libraries(LoggerUM PRIVATE
        SDL3::SDL3
        opengl32
        ${GOIO_SDK_DIR}/GoIO_DLL/Win64/GoIO_DLL.lib
)

add_custom_command(TARGET LoggerUM POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GOIO_SDK_DIR}/GoIO_DLL/Win64/GoIO_DLL.dll
        $<TARGET_FILE_DIR:LoggerUM>
)