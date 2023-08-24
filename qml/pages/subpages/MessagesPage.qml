import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../../components" as NeroshopComponents

Page {
    id: messagesPage
    background: Rectangle {
        color: "transparent"
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20; anchors.rightMargin: 40
        anchors.topMargin: 15; anchors.bottomMargin: anchors.topMargin
        RowLayout {
            // Side Panel
            Rectangle {
                id: sidePanel
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.preferredWidth: 300
                Layout.fillHeight: true
                color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#181a1b") : "#c9c9cd"//"#f0f0f0"
                clip: true
                border.color: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"
                radius: 7

                ListView {
                    id: senderList
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 5; anchors.bottomMargin: anchors.topMargin
                    width: 280
                    height: parent.height
                    spacing: 3
                    property var messages_model: User.getMessages()
                    model: ListModel {
                        Component.onCompleted: {
                            var uniqueSenderIds = [];
                            for (var i = 0; i < senderList.messages_model.length; i++) {
                                var senderId = senderList.messages_model[i].sender_id;
                                if (uniqueSenderIds.indexOf(senderId) === -1) {
                                    uniqueSenderIds.push(senderId);
                                    append({ sender_id: senderId });
                                }
                            }
                        }
                    }//50
                    delegate: Rectangle {
                        width: senderList.width
                        height: 60//40
                        color: "transparent"
                        radius: sidePanel.radius
                    
                        Rectangle {
                            anchors.fill: parent
                            color: parent.ListView.isCurrentItem ? ((NeroshopComponents.Style.darkTheme) ? ((NeroshopComponents.Style.themeName == "PurpleDust") ? "#3a3a46" : "#3c4143") : "#d7d7da") : "transparent"
                            radius: parent.radius
                        }
                    
                        Item {
                            anchors.fill: parent
                            RowLayout { // By default this should inherit its children rect size
                                anchors.left: parent.left
                                anchors.leftMargin: 10; 
                                anchors.rightMargin: anchors.leftMargin
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 10
                                Rectangle {
                                    id: senderAvatarRect
                                    Layout.preferredWidth: 32; Layout.preferredHeight: width
                                    border.color: parent.parent.parent.ListView.isCurrentItem ? "#7d6b9a" : "transparent"
                                    radius: 5
                                    Image {
                                        source: "https://api.dicebear.com/6.x/identicon/png?seed=%1".arg(modelData)
                                        anchors.centerIn: parent
                                        width: (parent.width - 8) - (senderAvatarRect.border.width * 2); height: width
                                        fillMode: Image.PreserveAspectFit
                                        mipmap: true
                                        asynchronous: true
                                    }
                                }
                                Text {
                                    id: senderNameIdText
                                    Layout.preferredWidth: parent.parent.parent.width - (parent.anchors.leftMargin + senderAvatarRect.width + parent.spacing + parent.anchors.rightMargin)
                                    text: Backend.getDisplayNameByUserId(modelData)//"Item " + (index + 1)
                                    elide: Text.ElideRight
                                    color: parent.parent.parent.ListView.isCurrentItem ? "#7d6b9a" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
                                    font.bold: parent.parent.parent.ListView.isCurrentItem ? true : false
                                    property string senderId: modelData
                                }
                            }
                        }
                    
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onEntered: {
                                parent.color = (NeroshopComponents.Style.darkTheme) ? ((NeroshopComponents.Style.themeName == "PurpleDust") ? "#3a3a46" : "#3c4143") : "#d7d7da"
                            }
                            onExited: {
                                parent.color = "transparent"
                            }
                            onClicked: {
                                // Handle item selection
                                //console.log("Selected Item " + (index + 1))
                                // Set current item
                                senderList.currentIndex = index
                                senderList.positionViewAtIndex(index, ListView.Contain)
                            }
                        }
                    }
                }
            }

            // Main Content
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: sidePanel.color//"#ffffff"
                border.color: sidePanel.border.color
                radius: sidePanel.radius
                clip: true

                Text {
                    visible: (senderList.count < 1)
                    anchors.centerIn: parent
                    text: "No messages found"
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                }
            
                ListView {
                    id: messageList
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 20; anchors.bottomMargin: anchors.topMargin
                    anchors.leftMargin: 20; anchors.rightMargin: anchors.leftMargin
                    spacing: 5
                    model: senderList.currentItem ? User.getMessages(senderList.currentItem.children[1].children[0].children[1].senderId, senderList.messages_model) : null
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 250 // Adjust the height as needed
                        color: "transparent"//index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
                        border.color: messageList.parent.border.color
                        radius: messageList.parent.radius
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                        
                            RowLayout {
                                spacing: 3
                                Text {
                                    id: senderNameText
                                    visible: (text !== modelData.sender_id)
                                    wrapMode: Text.WordWrap
                                    text: Backend.getDisplayNameByUserId(modelData.sender_id)
                                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                }
                                Text {
                                    id: senderIdText
                                    Layout.fillWidth: true // Fill the remaining available width
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                    text: senderNameText.visible ? ("(" + modelData.sender_id + ")") : modelData.sender_id
                                    color: "#4169e1"
                            
                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            navBar.uncheckAllButtons()
                                            pageStack.pushPageWithProperties("qrc:/qml/pages/ProfilePage.qml", { "messagesModel": {"sender_id": modelData.sender_id} })
                                        }
                                        onEntered: parent.color = "blue"
                                        onExited: parent.color = "#4169e1"
                                    }
                                }
                            }
                    
                            TextArea {
                                id: contentText
                                Layout.preferredWidth: parent.width//Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: modelData.content
                                wrapMode: Text.Wrap
                                readOnly: true
                                selectByMouse: true
                                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            }
                        }
                    }
                }
            }
        }
    }
}
