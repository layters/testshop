import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Item {
    id: banner
    implicitWidth: 300
    implicitHeight: 150

    default property alias content: view.contentData

    SwipeView {
        id: view
        clip: true
        anchors.fill: parent
        interactive: true
        orientation: Qt.Horizontal
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
}
