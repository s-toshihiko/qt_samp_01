import QtQuick 2.15
import QtQuick.Controls 2.15
import CustomVideo 1.0   // ← これを追加！

ApplicationWindow {
    visible: true
    width: 800
    height: 600
	x: 0  // 画面左上に配置
    y: 0
    title: "Qt GStreamer QML Sample"

    VideoItem {
        anchors.fill: parent
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 40
        color: "#00000088"
        Text {
            anchors.centerIn: parent
            text: "Overlay UI"
            color: "white"
        }
    }
}

