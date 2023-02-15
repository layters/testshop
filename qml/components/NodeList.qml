import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

//import FontAwesome 1.0
import "." as NeroshopComponents

Item {
    id: root
    property string selectedNode: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedNode
    property bool selectedNodeStatus: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedNodeStatus
    
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
                // Get monero nodes from settings.lua
                /*let network_type = Wallet.getNetworkTypeString()
                return Script.getTableStrings("neroshop.monero.nodes." + network_type)*/
                // Get monero nodes from https://monero.fail/health.json
                ////return Backend.getMoneroNodeList()
                // Get only stagenet nodes (for now)
                let stagenet_nodes = []
                const monero_node_list = Backend.getMoneroNodeList()
                for(let i = 0; i < monero_node_list.length; i++) {
                    if(monero_node_list[i].address.includes("38081") || monero_node_list[i].address.includes("38089")) {
                        stagenet_nodes.push(monero_node_list[i])
                    }
                }
                return stagenet_nodes;
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
                    }
                }

                RowLayout {
                    id: delegateRow
                    anchors.fill: parent
                    property string selectedNode: delegateRow.parent.ListView.isCurrentItem ? nodeAddressLabel.text : ""
                    property bool selectedNodeStatus: delegateRow.parent.ListView.isCurrentItem ? nodeStatusLabel.status : false

                    Label {
                        id: nodeStatusLabel
                        text: status ? "✅" : "❌"
                        color: status ? "#698b22" : "#dd4b4b"
                        Layout.maximumWidth: 25
                        property bool status: modelData.available
                    }

                    Label {
                        id: nodeAddressLabel
                        Layout.fillWidth: true
                        text: modelData.address//"node.neroshop.org:38081"//:18081"
                        color: delegateRow.parent.ListView.isCurrentItem ? "#471d00" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
                        font.bold: delegateRow.parent.ListView.isCurrentItem ? true : false
                        elide: Label.ElideRight
                    }

                    Label {
                        id: nodeHeightLabel
                        Layout.minimumWidth: heightTitle.width//50
                        Layout.maximumWidth: heightTitle.width//50
                        text: modelData.last_height
                        color: nodeAddressLabel.color
                        font.bold: nodeAddressLabel.font.bold
                        elide: Label.ElideRight
                    }
                }
            }
        }
    }
}
