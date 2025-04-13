cmake_minimum_required(VERSION 3.10)
project(EsscaleRS_Backend)

# Настройки для Windows
if(WIN32)
    # Отключаем поиск pthread на Windows
    set(THREADS_PREFER_PTHREAD_FLAG OFF)
    set(CMAKE_THREAD_PREFER_PTHREAD FALSE)
    
    # Устанавливаем тип рантайма (Dynamic Linking)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # Для Google Test
    option(gtest_force_shared_crt "Use shared (DLL) C runtime library" ON)
else()
    # Настройки для Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0135 NEW)

# Объявляем ОБЩУЮ библиотеку
add_library(backend_lib SHARED
    src/utils.cpp
)

# Указываем директории заголовков для backend_lib
target_include_directories(backend_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Главный исполняемый файл (заглушка)
add_executable(esscale_backend
    src/main.cpp
)
target_link_libraries(esscale_backend
    PRIVATE backend_lib
)

# Экспорт символов для Windows DLL
if(WIN32)
    target_compile_definitions(backend_lib PRIVATE BACKEND_LIB_EXPORT)
    set_target_properties(backend_lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Подключение тестов (после backend_lib)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

enable_testing()