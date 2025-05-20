cmake_minimum_required(VERSION 3.28)
project(QMusicPlayer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Attention: Dont forget to modify the CMAKE_PREFIX_PATH variable!
# TODO：请注意修改这里的路径！！
set(CMAKE_PREFIX_PATH "E:/Qt/6.8.1/msvc2022_64")
#set(CMAKE_PREFIX_PATH "E:/Qt/6.8.1/mingw_64")
list(APPEND CMAKE_PREFIX_PATH "./3rdParty/nlohnman_json")

# TODO: Taglib 路径也需要修改，这取决于你使用的是什么编译器和系统了！
set(TAGLIB_DIR "./3rdParty/taglib-2.0.2-MSVC")
#set(TAGLIB_DIR "./3rdParty/taglib-2.0.2-MinGW")
#set(TAGLIB_DIR "~/CPPLibs/taglib")
list(APPEND CMAKE_PREFIX_PATH ${TAGLIB_DIR})

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        MultimediaWidgets
        REQUIRED)
find_package(TagLib REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(QMusicPlayer main.cpp
        MusicPlayer.cpp
        MusicPlayer.h
        MusicPlayer.ui
        resource.qrc
        MusicPlayer.rc
        Player.cpp
        Player.h
        Player.ui
        AboutApp.cpp
        AboutApp.h
        AboutApp.ui
        playlist.cpp
        playlist.h
        AppConfig.h
        Options.cpp
        Options.h
        Options.ui
)

target_link_libraries(QMusicPlayer
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Multimedia
        Qt::MultimediaWidgets
        TagLib::TagLib
        nlohmann_json::nlohmann_json
)

# Copy Taglib.dll to binary directory
if (${TAGLIB_DIR})
    file(${TAGLIB_DIR}/bin/tag.dll DESTINATION ${CMAKE_BINARY_DIR})
endif()

# 仅在 Windows 平台且为 Release 模式时禁用命令提示符
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    # 设置为 Windows GUI 应用程序（无命令提示符）
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
    )

    # 针对 MinGW 编译器的额外设置
    if(MINGW)
        set_target_properties(${PROJECT_NAME} PROPERTIES
                LINK_FLAGS "-Wl,-subsystem,windows"
        )
    endif()
endif()
