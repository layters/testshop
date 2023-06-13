import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Item {
    id: banner
    implicitWidth: 300
    implicitHeight: 300//150

    property int rotationInterval: 3000 // Rotation interval in milliseconds

    default property alias content: view.contentData

    SwipeView {
        id: view
        clip: true
        anchors.fill: parent
        interactive: true
        orientation: Qt.Horizontal
        currentIndex: indicator.currentIndex
    }

    PageIndicator {
        id: indicator
        anchors {
            bottom: view.bottom
            horizontalCenter: parent.horizontalCenter
        }
        interactive: true
        count: view.count
        currentIndex: view.currentIndex
    }
    
    Timer {
        id: rotationTimer
        interval: banner.rotationInterval
        repeat: true
        running: true

        onTriggered: {
            // Increment the currentIndex to rotate the banner
            view.currentIndex = (view.currentIndex + 1) % view.count;
        }
    }    
}
