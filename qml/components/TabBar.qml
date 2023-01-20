import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Item {
    id: tabBar
    property real radius: 3;
    property alias currentButton: tabButtonGroup.checkedButton
    property int currentIndex: tabButtonGroup.checkedButton.buttonIndex
    property real buttonWidth: 200
    property real buttonHeight: 50
    property string color0: "royalblue"
    property string color1: "#e0e0e0"
    property alias model: tabButtonRepeater.model
    property var buttonAt: tabButtonRepeater.itemAt

ButtonGroup {
    id: tabButtonGroup
    buttons: column.children
    exclusive: true // only one button in the group can be checked at any given time
    onClicked: {
        console.log("Switched to", button.text + " (index: " + button.buttonIndex + ")")
        button.checked = true
    }
}

Row {//TODO: try using a Column too
    id: column
    spacing: 2
    anchors.centerIn: parent

    Repeater {
        id: tabButtonRepeater
        model: null
        delegate: Button {
            checked: true
            text: modelData
            ButtonGroup.group: tabButtonGroup // attaches a button to a button group
            property int buttonIndex: index
            //display: AbstractButton.IconOnly
            //icon.source: "qrc:/images/grid.png"
            //icon.color: !this.checked ? "#39304f" : "#ffffff"// icon color is set automatically unless we set it ourselves, which we do here
            width: tabBar.buttonWidth; height: tabBar.buttonHeight
            background: Rectangle {
                radius: tabBar.radius
                color: parent.checked ? color0 : color1
            }
            contentItem: Text {
                text: parent.text
                font.bold: parent.checked ? true : false
                color: parent.checked ? "#ffffff" : "#000000"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}

}
