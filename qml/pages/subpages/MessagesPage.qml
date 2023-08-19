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
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: User.getMessages()
            delegate: Rectangle {
                width: parent.width
                height: 250
                color: index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
                ColumnLayout {
                    anchors.fill: parent
                    Text {
                        id: senderText
                        text: modelData.sender_id
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
                    TextArea {
                        id: contentText
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: modelData.content
                        wrapMode: Text.Wrap
                        readOnly: true
                        selectByMouse: true
                    }
                }
            }
        }
    }
}
