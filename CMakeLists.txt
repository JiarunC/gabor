cmake_minimum_required(VERSION 3.16)

project(WAV VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:\\Qt\\6.12.0\\mingw_64\\lib\\cmake")

find_package(Qt6 REQUIRED COMPONENTS Quick Gui Sql Core QuickControls2 Concurrent)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appWAV
    main.cpp
)

qt_add_qml_module(appWAV
    URI WAV
    QML_FILES
        Main.qml
        SOURCES
        config.h
)

add_subdirectory(Modules/Overlay)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appWAV PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appWAV
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appWAV
    PRIVATE Qt6::Gui Qt6::Sql Qt6::Core Qt6::Quick Qt6::QuickControls2 Qt6::Concurrent Overlaylibplugin
)

install(TARGETS appWAV
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)