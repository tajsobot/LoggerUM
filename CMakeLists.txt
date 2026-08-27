cmake_minimum_required(VERSION 4.2)
project(LoggerUM)

set(CMAKE_CXX_STANDARD 20)

set(GOIO_SDK_DIR ${CMAKE_SOURCE_DIR}/libraries/GoIO/redist)

add_executable(LoggerUM main.cpp)

target_include_directories(LoggerUM PRIVATE ${GOIO_SDK_DIR}/include)
target_link_libraries(LoggerUM PRIVATE ${GOIO_SDK_DIR}/GoIO_DLL/Win64/GoIO_DLL.lib)

add_custom_command(TARGET LoggerUM POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GOIO_SDK_DIR}/GoIO_DLL/Win64/GoIO_DLL.dll
        $<TARGET_FILE_DIR:LoggerUM>
)

target_compile_definitions(LoggerUM PRIVATE TARGET_OS_WIN)