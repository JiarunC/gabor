cmake_minimum_required(VERSION 4.4.2)

project(WAV VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:\\Qt\\6.12.0\\msvc2022_64\\lib\\cmake")

include(InstallRequiredSystemLibraries)

find_package(Qt6 REQUIRED COMPONENTS Quick Gui Sql Core QuickControls2 Concurrent)

set(MKL_LINK "static")
set(MKL_THREADING "sequential")

find_package(MKL CONFIG REQUIRED PATHS $ENV{MKLROOT})

qt_standard_project_setup(REQUIRES 6.12)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/rc/icons/Resource.rc")
qt_add_executable(appWAV
    main.cpp ${app_icon_resource_windows}
)
target_compile_options(appWAV PUBLIC "/Zc:__cplusplus")
add_compile_options(/Zc:__cplusplus)

qt_add_qml_module(appWAV
    URI WAV
    QML_FILES
        Main.qml
        CustomComboBox.qml
    RESOURCES
        rc/images/expand_all.png
        rc/icons/wave.ico
        rc/fonts/Alice/Alice-Regular.ttf
        rc/fonts/Baumans/Baumans-Regular.ttf
)

add_subdirectory(Modules/Overlay)
add_subdirectory(Modules/Config)

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
    PRIVATE Qt6::Gui Qt6::Sql Qt6::Core Qt6::Quick Qt6::QuickControls2 Qt6::Concurrent Overlaylibplugin Configlibplugin PUBLIC MKL::MKL
)

install(TARGETS appWAV
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
        TARGET appWAV
        OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})

include(CPack)