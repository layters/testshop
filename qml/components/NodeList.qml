import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

//import FontAwesome 1.0
import "." as NeroshopComponents

Item {
    id: root
    property string selectedNode: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedNode
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            spacing: 2
            property real titleBoxRadius: 3

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 25
                color: "#6c6c6f"
                radius: parent.titleBoxRadius

                Label {
                    text: qsTr("Node")
                    color: "white"
                    anchors {
                        horizontalCenter: parent.horizontalCenter//left: parent.left;leftMargin: 4
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            Rectangle {
                id: heightTitle
                Layout.minimumWidth: 100//50
                Layout.minimumHeight: 25
                color: "#6c6c6f"
                radius: parent.titleBoxRadius

                Label {
                    text: qsTr("Height")
                    color: "white"
                    anchors {
                        horizontalCenter: parent.horizontalCenter//left: parent.left; leftMargin: 4
                        verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        ListView {
            id: listView
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical: ScrollBar { }
            model: {
                let network_type = moneroNetworkTypeBox.displayText.toLowerCase()
                return Script.getTableStrings("neroshop.monero.nodes." + network_type)
                //Todo: replace Script.getTableStrings() with Backend.getNodeList() when app is released with mainnet
                //return Backend.getNodeList() // <- This will only work for mainnet nodes
            }//50
            delegate: Item {
                width: listView.width
                height: 25

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
                    property string selectedNode: delegateRow.parent.ListView.isCurrentItem ? nodeAddressLabel.text : ""

                    Label {
                        id: nodeStatusLabel
                        text: index % 3 === 0 ? "✅" : "❌" // TODO create normal model
                        Layout.maximumWidth: 25
                        property bool status: index % 3 === 0//false // todo: use data from model to determine the status
                    }

                    Label {
                        id: nodeAddressLabel
                        Layout.fillWidth: true
                        text: modelData//"node.neroshop.org:38081"//:18081"
                        color: delegateRow.parent.ListView.isCurrentItem ? "#471d00" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
                        font.bold: delegateRow.parent.ListView.isCurrentItem ? true : false
                        elide: Label.ElideRight
                    }

                    Label {
                        id: nodeHeightLabel
                        Layout.minimumWidth: heightTitle.width//50
                        Layout.maximumWidth: heightTitle.width//50
                        text: "1243821" // TODO create normal model
                        color: nodeAddressLabel.color
                        font.bold: nodeAddressLabel.font.bold
                        elide: Label.ElideRight
                    }
                }
            }
        }
    }
}
