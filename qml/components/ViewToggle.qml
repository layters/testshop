import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Item {
    id: viewToggle
    property real radius: 3;
    property alias currentButton: viewButtonGroup.checkedButton
    property int currentIndex: viewButtonGroup.checkedButton.buttonIndex
    width: childrenRect.width; height: childrenRect.height

ButtonGroup {
    id: viewButtonGroup
    buttons: column.children
    exclusive: true // only one button in the group can be checked at any given time
    onClicked: {
        console.log("Switched to", button.text)
        button.checked = true
    }
}

Row {
    id: column
    spacing: 2

    Button {
        checked: true
        text: qsTr("Grid view")
        ButtonGroup.group: viewButtonGroup // attaches a button to a button group
        property int buttonIndex: 0
        display: AbstractButton.IconOnly
        icon.source: "qrc:/images/grid.png"
        icon.color: !this.checked ? "#39304f" : "#ffffff"// icon color is set automatically unless we set it ourselves, which we do here
        background: Rectangle {
            radius: viewToggle.radius
            color: parent.checked ? "#39304f" : "#e0e0e0"//"#353637" : "#e0e0e0"// rgb(53, 54, 55), rgb(224, 224, 224);
        }              
    }

    Button {
        text: qsTr("List view")
        ButtonGroup.group: viewButtonGroup
        property int buttonIndex: 1
        display: AbstractButton.IconOnly
        icon.source: "qrc:/images/list.png"
        icon.color: !this.checked ? "#39304f" : "#ffffff"
        background: Rectangle {
            radius: viewToggle.radius
            color: parent.checked ? "#39304f" : "#e0e0e0"//"#353637" : "#e0e0e0"
        }
    }
}

}
