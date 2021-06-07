cmake_minimum_required(VERSION 3.16.0)

# It is typically better to pass this in.  "-DCMAKE_PREFIX_PATH=..."
# set(CMAKE_PREFIX_PATH ~/Qt/5.12.8/clang_64)

project(terminal VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# This code keeps a terminal window from being generated when opened.
# This could be used for debug (by disabling it)
# Set some Win32 Specific Settings
IF(WIN32)
    SET(GUI_TYPE WIN32)
ENDIF(WIN32)
# Set some Apple MacOS Specific settings
IF (APPLE)
    SET(GUI_TYPE MACOSX_BUNDLE)
ENDIF (APPLE)

find_package(   Qt5 COMPONENTS 
                Widgets REQUIRED
                SerialPort REQUIRED)

add_executable(terminal
    ${GUI_TYPE}
    mainwindow.ui
    mainwindow.cpp
    console.cpp
    settingsdialog.cpp
    settingsdialog.ui
    main.cpp
    terminal.qrc
)

target_link_libraries(terminal 
    Qt5::Widgets 
    Qt5::SerialPort)