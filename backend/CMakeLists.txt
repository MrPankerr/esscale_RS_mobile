cmake_minimum_required(VERSION 3.10)
project(EsscaleRS_Backend)

# Условные флаги компилятора
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
endif()

set(CMAKE_CXX_STANDARD 17)

# Объявляем ОБЩУЮ библиотеку
add_library(backend_lib STATIC
    src/utils.cpp
)

# Главный исполняемый файл (заглушка)
add_executable(esscale_backend
    src/main.cpp
)
target_link_libraries(esscale_backend
    PRIVATE backend_lib
)

# Подключение тестов
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

enable_testing()