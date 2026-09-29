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

# Post-codegen fixes: codegen sometimes emits REX_FATAL for branch targets
# that do exist. Apply manual patches after codegen runs.
if(TARGET condemned2recomp_codegen)
    add_custom_target(condemned2recomp_fix_codegen
        COMMAND ${CMAKE_COMMAND} -E echo "Applying codegen fixes..."
        COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/fix-codegen.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Patching codegen output for unresolved branches"
        VERBATIM
    )
    add_dependencies(condemned2recomp_fix_codegen condemned2recomp_codegen)
    # The recomp target must wait for our fixes
    if(TARGET condemned2recomp_recomp)
        add_dependencies(condemned2recomp_recomp condemned2recomp_fix_codegen)
    endif()
endif()
