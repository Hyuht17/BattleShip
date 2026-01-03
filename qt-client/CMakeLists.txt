cmake_minimum_required(VERSION 3.16)
project(BattleshipClient VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable automoc for Qt
set(CMAKE_AUTOMOC ON)

# C Client Library
add_library(battleship_client STATIC
    ../client-lib/battleship_client.c
    ../client-lib/battleship_client.h
)

target_include_directories(battleship_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../client-lib
)

# Qt Application
set(SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    GameClient.cpp
    GameClient.h
    ConnectionWidget.cpp
    ConnectionWidget.h
    LoginWidget.cpp
    LoginWidget.h
    LobbyWidget.cpp
    LobbyWidget.h
    GameWidget.cpp
    GameWidget.h
)

add_executable(battleship_qt ${SOURCES})

target_link_libraries(battleship_qt
    Qt6::Core
    Qt6::Widgets
    battleship_client
)

target_include_directories(battleship_qt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../client-lib
)

# Install
install(TARGETS battleship_qt
    RUNTIME DESTINATION bin
)
