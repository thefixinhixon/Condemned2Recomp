cmake_minimum_required(VERSION 3.28)
project(condemned2_launcher CXX)

set(CMAKE_AUTOMOC ON)

# Allow standalone builds (find Qt6 if the parent project hasn't).
if(NOT TARGET Qt6::Core)
  find_package(Qt6 6.4 COMPONENTS Core Gui Widgets REQUIRED)
endif()

add_executable(condemned2_launcher
    main.cpp
    main_window.cpp
    main_window.h
    game_config.cpp
    game_config.h
    import_service.cpp
    import_service.h
    keybind_button.cpp
    keybind_button.h
)

target_link_libraries(condemned2_launcher PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

target_compile_features(condemned2_launcher PRIVATE cxx_std_23)
target_compile_definitions(condemned2_launcher PRIVATE
    CONDEMNED2_LAUNCHER_VERSION="0.1.0"
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    target_compile_options(condemned2_launcher PRIVATE -march=armv8.4-a)
endif()

include(GNUInstallDirs)
install(TARGETS condemned2_launcher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(CONDEMNED2_LAUNCHER_BUILD_TESTS)
    find_package(Qt6 6.4 COMPONENTS Test QUIET)
    if(TARGET Qt6::Test)
        add_executable(launcher_config_test
            tests/config_test.cpp
            game_config.cpp
            game_config.h
            import_service.cpp
            import_service.h
            keybind_button.cpp
            keybind_button.h
        )
        target_include_directories(launcher_config_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(launcher_config_test PRIVATE Qt6::Core Qt6::Test)
        add_test(NAME launcher_config_test COMMAND launcher_config_test)
    endif()
endif()
