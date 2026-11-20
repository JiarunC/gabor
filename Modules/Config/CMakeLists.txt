project(Config VERSION 0.1 LANGUAGES CXX)

qt_add_qml_module(${PROJECT_NAME}lib
        STATIC
        URI Config
        SOURCES
        Config.h
)

target_link_libraries(${PROJECT_NAME}lib
        PRIVATE Qt6::Gui Qt6::Core
)