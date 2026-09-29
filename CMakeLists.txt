# condemned2recomp - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(condemned2recomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ReXGlue SDK's memory.cpp uses SSSE3 intrinsics.
if(NOT WIN32)
    add_compile_options(-mssse3 -msse4.1)
endif()

include(generated/rexglue.cmake)

# Sources
set(CONDEMNED2RECOMP_SOURCES
    src/main.cpp
)

if(WIN32)
    add_executable(condemned2recomp WIN32 ${CONDEMNED2RECOMP_SOURCES})
else()
    add_executable(condemned2recomp ${CONDEMNED2RECOMP_SOURCES})
endif()

rexglue_setup_target(condemned2recomp)

# imgui headers ship inside the SDK tree but aren't added to the include
# path by rexglue_setup_target; add them explicitly (same as hells-gate).
if(REXSDK_DIR)
    target_include_directories(condemned2recomp PRIVATE
        "${REXSDK_DIR}/thirdparty/imgui"
    )
endif()

if(TARGET imgui::imgui)
    target_link_libraries(condemned2recomp PRIVATE imgui::imgui)
elseif(TARGET imgui)
    target_link_libraries(condemned2recomp PRIVATE imgui)
endif()

# Qt launcher (graphics/audio/locale settings, ISO import, game finder).
option(CONDEMNED2_BUILD_LAUNCHER "Build the Qt launcher" ON)
if(CONDEMNED2_BUILD_LAUNCHER)
    add_subdirectory(launcher)
endif()
