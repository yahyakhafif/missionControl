cmake_minimum_required(VERSION 3.16)
project(MissionControl VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 QUIET COMPONENTS Core Widgets Network)
if(Qt6_FOUND)
    set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::Network)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
    set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Network)
    message(STATUS "Using Qt5")
endif()

set(SOURCES
    src/main.cpp
    src/triangle/mainwindow.cpp
    src/battery/mapwidget.cpp
    src/signals/roverworker.cpp
)

set(HEADERS
    src/triangle/mainwindow.h
    src/battery/mapwidget.h
    src/signals/roverworker.h
    src/window/roverstate.h
)

add_executable(MissionControl ${SOURCES} ${HEADERS})

target_link_libraries(MissionControl PRIVATE ${QT_LIBS})
target_include_directories(MissionControl PRIVATE
    src
    src/triangle
    src/battery
    src/signals
    src/window
)

if(APPLE)
    set_target_properties(MissionControl PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.missioncontrol.rover"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    )
endif()