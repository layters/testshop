import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

import "." as NeroshopComponents

Item {
    id: root
    property string selectedNode: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedNode
    property bool selectedNodeStatus: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedNodeStatus
    property alias currentIndex: listView.currentIndex
    property alias model: listView.model
    
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
                // Get all monero nodes from https://monero.fail/health.json
                const monero_node_list = (Wallet.getWalletType() == 1) ? Backend.getNodeList("wownero") : Backend.getNodeList("monero")
                return monero_node_list
            }
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
                        settingsDialog.save()
                    }
                }

                RowLayout {
                    id: delegateRow
                    anchors.fill: parent
                    property string selectedNode: delegateRow.parent.ListView.isCurrentItem ? nodeAddressLabel.text : ""
                    property bool selectedNodeStatus: delegateRow.parent.ListView.isCurrentItem ? nodeStatusLabel.status : false

                    Label {
                        id: nodeStatusLabel
                        text: (typeof modelData === "string") ? qsTr("\uf1ce") : (status ? qsTr("\uf14a") : qsTr("\uf00d"))//"✅" : "❌"
                        color: (typeof modelData === "string") ? "royalblue" : (status ? "#698b22" : "#dd4b4b")
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        Layout.maximumWidth: 25
                        property bool status: (typeof modelData === "string") ? false : modelData.available
                    }

                    Label {
                        id: nodeAddressLabel
                        Layout.fillWidth: true
                        text: (typeof modelData === "string") ? modelData : modelData.address//"node.neroshop.org:38081"//:18081"
                        color: delegateRow.parent.ListView.isCurrentItem ? "#471d00" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
                        font.bold: delegateRow.parent.ListView.isCurrentItem ? true : false
                        elide: Label.ElideRight
                    }

                    Label {
                        id: nodeHeightLabel
                        Layout.minimumWidth: heightTitle.width//50
                        Layout.maximumWidth: heightTitle.width//50
                        text: (typeof modelData === "string") ? "- -" : modelData.last_height
                        color: nodeAddressLabel.color
                        font.bold: nodeAddressLabel.font.bold
                        elide: Label.ElideRight
                    }
                }
            }
        }
    }
}
