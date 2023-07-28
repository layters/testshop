import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../../components" as NeroshopComponents

Page {
    id: messagesPage
    background: Rectangle {
        color: "transparent"
    }
    property var model: User.getMessages()
    
    ColumnLayout {
        Text {
            id: content
            text: messagesPage.model[0].content
        }
        Text {
            id: sender_id
            text: messagesPage.model[0].sender_id
        }
    }
}
