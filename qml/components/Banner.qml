import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12

// Banner will be changed more frequently than the other components and will contain the latest news from Monero news sources, featured items, product deals, etc.
Item {
    id: banner
    implicitWidth: 300; implicitHeight: 150 // default size
    property real radius: 0
    // Reminder: SwipeView is non-visual, only the Rectangles are!
    SwipeView { 
        id: view
        //implicitWidth: parent.width
        //implicitHeight: parent.height
    /*Repeater {
        model: 6
        Loader {
            active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
            sourceComponent: Text {
                text: index
                Component.onCompleted: console.log("created:", index)
                Component.onDestruction: console.log("destroyed:", index)
            }
        }
    }*/
        currentIndex: indicator.currentIndex//0
        anchors.fill: parent
        interactive: true
        orientation: Qt.Horizontal
        //property alias currentItem: 

        Rectangle {
            id: firstPage
            color: "blue"
            width: parent.parent.width; height: parent.parent.height//width: 500; height: 500
            radius: banner.radius
        }
        Rectangle {
            id: secondPage
            color: "red"
            width: parent.parent.width; height: parent.parent.height//width: 500; height: 500        
            radius: banner.radius
        }
        Rectangle {
            id: thirdPage
            color: "purple"
            width: parent.parent.width; height: parent.parent.height//width: 500; height: 500        
            radius: banner.radius
        }    
    }      

    PageIndicator {
        id: indicator
        interactive: true
        count: view.count
        currentIndex: view.currentIndex

        anchors.bottom: view.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
