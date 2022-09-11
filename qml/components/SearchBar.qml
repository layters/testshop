import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Item {
    TextField {
        id: search_bar
        color: "#ffffff" // textColor
        width: 400; height: 40
        selectByMouse: true
        
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        
        background: Rectangle { 
            color: "#050506"
        }
    }

    Button {
        id: search_button
        text: qsTr("Search")
        //onClicked: 
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        anchors.left: search_bar.right
        anchors.leftMargin: 1
        anchors.top: search_bar.top
        width: 50; height: search_bar.height
        
        icon.source: neroshopResourceDir + "/search.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#40404f"
        }        
    }
}
