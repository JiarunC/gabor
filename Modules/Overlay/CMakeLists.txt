project(Overlay VERSION 0.1 LANGUAGES CXX)

qt_add_qml_module(${PROJECT_NAME}lib
        STATIC
        URI Overlay
        QML_FILES
        Overlay.qml
        SOURCES
        AudioCapture.h
        AudioProcess.h
)

target_link_libraries(${PROJECT_NAME}lib
        PRIVATE Qt6::Gui Qt6::Core Qt6::Concurrent
)