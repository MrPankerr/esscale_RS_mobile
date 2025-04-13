# Настройки рантайма для Windows
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    option(gtest_force_shared_crt "Use shared (DLL) C runtime library" ON)
endif()

# Автоматическая загрузка Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# Тестовый бинарник
add_executable(test_utils
    test_utils.cpp
)

# Копируем backend_lib.dll в директорию с test_utils.exe для Windows
if(WIN32)
    add_custom_command(TARGET test_utils POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:backend_lib>
        $<TARGET_FILE_DIR:test_utils>
    )
endif()

# Связь тестов с библиотекой и Google Test
target_link_libraries(test_utils
    PRIVATE backend_lib
    PRIVATE gtest_main
)

# Для совместимости с Windows
if(WIN32)
    set_target_properties(test_utils PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

enable_testing()

include(GoogleTest)
gtest_discover_tests(test_utils
  PROPERTIES
    TIMEOUT 10
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)