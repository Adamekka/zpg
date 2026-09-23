cmake_minimum_required(VERSION 4.4)

# MARK: Project

project(zpg LANGUAGES C CXX)

# MARK: Tooling

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" ON)

# MARK: Warnings

set(CXX_FLAGS
    -fno-exceptions
    -Wall
    -Wcast-align
    -Wextra
    -Wformat
    -Wformat-security
    -Wnull-dereference
    -Wredundant-decls
    -Wreturn-type
    -Wstrict-overflow
    -Wuninitialized
    -Wvla
    -fstack-protector-strong
)

if(ENABLE_SANITIZERS)
    set(SANITIZER_FLAGS
        -fsanitize=address
        -fsanitize=undefined
    )
endif()

# MARK: Target

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE ${CXX_FLAGS})

if(ENABLE_SANITIZERS)
    target_compile_options(${PROJECT_NAME} PUBLIC ${SANITIZER_FLAGS})
    target_link_options(${PROJECT_NAME} PUBLIC ${SANITIZER_FLAGS})
endif()

# glad
add_library(glad STATIC ${PROJECT_SOURCE_DIR}/deps/glad/src/gl.c)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/deps/glad/include)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

# glfw
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# glm
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
