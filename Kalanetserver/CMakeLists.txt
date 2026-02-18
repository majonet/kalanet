cmake_minimum_required(VERSION 3.16)
project(KalaNet VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
# If Qt6 not found, try Qt5
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets Network)
endif()

set(SOURCES
    src/main.cpp
    src/Product.cpp
    src/User.cpp
    src/DataManager.cpp
    src/LoginDialog.cpp
    src/MainWindow.cpp
)

set(HEADERS
    include/Product.h
    include/User.h
    include/DataManager.h
    include/LoginDialog.h
    include/MainWindow.h
)

add_executable(KalaNet ${SOURCES} ${HEADERS})

target_include_directories(KalaNet PRIVATE include)

if(Qt6_FOUND)
    target_link_libraries(KalaNet PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
    )
else()
    target_link_libraries(KalaNet PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
    )
endif()

# Create data directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
