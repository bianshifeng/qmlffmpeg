import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Bian 1.0
import QtMultimedia 5.5

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        Page1 {
        }

        Page {

            VideoOutput{
                anchors.fill: parent
                source:id_player
            }

            FFPlayer{
                id: id_player
                source: "rtsp://localhost:8554/"

            }

            Button{
                text: "sdfd"
                anchors.centerIn: parent
                onClicked: id_player.play()
            }
        }
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("First")
        }
        TabButton {
            text: qsTr("Second")
        }
    }
}
