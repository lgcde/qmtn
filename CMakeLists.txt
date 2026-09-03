cmake_minimum_required(VERSION 3.16)
project(qmtn VERSION 0.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Network Widgets Svg REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/mtnworker.cpp
    src/mtnworker.h
    src/mtnjob.cpp
    src/mtnjob.h
    src/imageitemview.cpp
    src/imageitemview.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/settingsdialog.ui
    src/fileselector.cpp
    src/fileselector.h
    src/iconprovider.cpp
    src/iconprovider.h
    src/settingsdata.cpp
    src/settingsdata.h
    src/profilemodel.cpp
    src/profilemodel.h
    src/images.qrc
    icons/qmtn.ico
)

add_executable(qmtn ${PROJECT_SOURCES})

target_include_directories(qmtn PRIVATE src src/plugins src/uploaders)

target_link_libraries(qmtn PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
    Qt6::Svg
)

target_compile_definitions(qmtn PRIVATE
    VERSION_RELEASE="\"0.5\""
    VERSION_FROM_GIT_TAG="\"0.5\""
)
