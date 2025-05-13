cmake_minimum_required(VERSION 3.30)
project(QMusicPlayer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Attention: Dont forget to modify the CMAKE_PREFIX_PATH variable!
# 请注意修改这里的两个路径！！
set(CMAKE_PREFIX_PATH "E:/Qt/6.8.1/msvc2022_64")
list(APPEND CMAKE_PREFIX_PATH "./3rdParty/taglib-2.0.2")  # Complied by MSBuild (MSVC)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        MultimediaWidgets
        REQUIRED)
find_package(TagLib REQUIRED)

add_executable(QMusicPlayer main.cpp
        MusicPlayer.cpp
        MusicPlayer.h
        MusicPlayer.ui
        resource.qrc
        MusicPlayer.rc
        Player.cpp
        Player.h
        Player.ui
)

target_link_libraries(QMusicPlayer
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Multimedia
        Qt::MultimediaWidgets
        TagLib::TagLib
)

