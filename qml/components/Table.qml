import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

import "." as NeroshopComponents

Item {
    id: table
    ColumnLayout {
        anchors.fill: parent
        //spacing: 2
        property real titleBoxRadius
        
        RowLayout {
            Layout.fillWidth: true
            //spacing: 2
            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 50
                color: "transparent"//"#6c6c6f"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                radius: parent.parent.titleBoxRadius
                Label {
                    text: qsTr("Order #")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 50
                color: "transparent"//"#6c6c6f"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                radius: parent.parent.titleBoxRadius
                Label {
                    text: qsTr("Date")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 50
                color: "transparent"//"#6c6c6f"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                radius: parent.parent.titleBoxRadius
                Label {
                    text: qsTr("Customer Name/ID")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }  
            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 50
                color: "transparent"//"#6c6c6f"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                radius: parent.parent.titleBoxRadius
                Label {
                    text: qsTr("Status") // Received, Pending, Completed
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }  
            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 50
                color: "transparent"//"#6c6c6f"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                radius: parent.parent.titleBoxRadius
                Label {
                    text: qsTr("Actions") // Approve, Cancel
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }    
            /*Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 50
                color: "transparent"//"#6c6c6f"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                radius: parent.parent.titleBoxRadius
                Label {
                    text: qsTr("?")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }*/                                                          
        }
        
        ListView {
            id: listView
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical: ScrollBar { }
            model: 5
            delegate: Rectangle {
                width: listView.width
                height: 100
                color: "transparent"
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                Rectangle {
                    anchors.fill: parent
                    color: parent.ListView.isCurrentItem ? "#ff8b3d" : "transparent"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        listView.currentIndex = index
                        listView.positionViewAtIndex(index, ListView.Contain)
                    }
                }                
                
                RowLayout {
                    id: delegateRow
                    anchors.fill: parent
                    
                    Label {
                        id: orderNumberLabel
                        text: qsTr("Test")
                        color: "white"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        Layout.maximumWidth: 25
                    }     
                    Label {
                        text: qsTr("Test")
                        color: "white"
                        font.bold: true
                        Layout.maximumWidth: 25
                    }    
                    Label {
                        text: qsTr("Test")
                        color: "white"
                        font.bold: true
                        //Layout.maximumWidth: 25
                    }                                                          
                }                
            }
        }
    } 
}
