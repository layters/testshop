// consists of the navigational bar with menus and search bar
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "../components" as NeroshopComponents // Tooltip

Page {
    id: homePage
    background: Rectangle {
        color: "transparent" // Make transparent to blend in with theme
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: 20        
        ScrollBar.vertical.policy: ScrollBar.AsNeeded//ScrollBar.AlwaysOn
        clip: true
        
        GridLayout {
            anchors.fill: parent
            NeroshopComponents.Banner {
                id: topBanner
                Layout.preferredWidth: scrollView.width//width: scrollView.width//homePage.width // will not fill page if you use parent.width. Use scrollView.width instead
                Layout.row: 0 // sets the row position
                //anchors.horizontalCenter: scrollView.horizontalCenter// Centered in parent horizontally (x-axis) // To fill entire homePage screen use: anchors.fill: parent
                radius: 15
            }
        }
    } // ScrollView
}
