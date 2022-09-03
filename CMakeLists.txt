cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)

project(Template
        VERSION 0.0.1
        LANGUAGES C CXX)


set(LOCAL_EXECUTABLE_NAME "InfiniSweeper")
# Source files (just grab all of them)
file(GLOB SOURCES CONFIGURE_DEPENDS
    "src/*.h"
    "src/*.hpp"
    "src/*.cpp"
)

# Compiler definitions
set(DEFINES
)

# Compiler options
set(OPTIONS
)

# LTO magic and fast math when releasing
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if(supported)
        cmake_policy(SET CMP0069 NEW) 
        set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options("/fp:fast")
    else()
        add_compile_options("-ffast-math")
    endif()
endif()


add_executable(${LOCAL_EXECUTABLE_NAME})
target_sources(${LOCAL_EXECUTABLE_NAME} PRIVATE ${SOURCES})
target_compile_definitions(${LOCAL_EXECUTABLE_NAME} PRIVATE ${DEFINES})
target_compile_options(${LOCAL_EXECUTABLE_NAME} PRIVATE ${OPTIONS})

if(WIN32)
    target_sources(${LOCAL_EXECUTABLE_NAME} PRIVATE "res/icon.rc")
endif()

# On Windows, disable console on release. -mwindows does not work in Clang with GCC-like commandline mode but same argument as MSVC mode works
if(WIN32)
    if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            target_link_options(${LOCAL_EXECUTABLE_NAME} PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_link_options(${LOCAL_EXECUTABLE_NAME} PRIVATE "-Wl,-subsystem:windows,-entry:mainCRTStartup")
        else()# GCC only
            target_link_options(${LOCAL_EXECUTABLE_NAME} PRIVATE "-mwindows")
        endif()
    endif()
endif()

set_target_properties(${LOCAL_EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")

# Libraries

set(ADDITIONAL_INCLUDES
"imgui/"
"rlImGui/"
"raylib/src/external/glfw/deps/"
"tomlplusplus/include/toml++/"
)

list(TRANSFORM ADDITIONAL_INCLUDES PREPEND "vendor/")
target_include_directories(${LOCAL_EXECUTABLE_NAME} PRIVATE ${ADDITIONAL_INCLUDES})

set(LOCAL_SUBDIRECTORIES 
"raylib" 
"raylib-cpp"
)

list(TRANSFORM LOCAL_SUBDIRECTORIES PREPEND "vendor/")
list(TRANSFORM LOCAL_SUBDIRECTORIES APPEND "/")

set(LIBRARIES
"raylib" 
"raylib_cpp"
)

target_link_libraries(${LOCAL_EXECUTABLE_NAME} PRIVATE ${LIBRARIES})

foreach(SUBDIRECTORY ${LOCAL_SUBDIRECTORIES})
    add_subdirectory(${SUBDIRECTORY})
endforeach()

# imgui & rlImGui, simply compile together with project

file(GLOB IMGUI CONFIGURE_DEPENDS
    "vendor/imgui/*.h"
    "vendor/imgui/*.hpp"
    "vendor/imgui/*.cpp"
    "vendor/rlImGui/*.h"
    "vendor/rlImGui/*.hpp"
    "vendor/rlImGui/*.cpp"
)

target_sources(${LOCAL_EXECUTABLE_NAME} PRIVATE ${IMGUI})
