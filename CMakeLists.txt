cmake_minimum_required(VERSION 3.16)
project(QtGStreamerQML LANGUAGES C CXX)

# ✅ これを追加！（自動 MOC/UIS/RCC を有効にする）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt モジュールの読み込み
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick OpenGL)

# GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0)

# 実行可能ファイルを定義
add_executable(qtgstqml
    main.cpp
    VideoItem.cpp
    VideoItem.h
    VideoRenderer.cpp
    VideoRenderer.h
    GStreamerGrabber.cpp
    GStreamerGrabber.h
    qml.qrc
)

# ヘッダーに Q_OBJECT マクロが含まれている場合、moc が必要
# CMAKE_AUTOMOC を ON にするだけで通常は OK

# インクルードとリンク
target_include_directories(qtgstqml PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(qtgstqml
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::OpenGL
    ${GSTREAMER_LIBRARIES}
)

