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
    property alias count: listView.count
    property alias list: listView
    
    BusyIndicator {
        id: busyIndicator
        running: WalletNodeProvider.loadingNodes
        visible: WalletNodeProvider.loadingNodes
        anchors.centerIn: parent
    }
    
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
            model: WalletNodeProvider.nodes//{
                // Get all monero nodes from https://monero.fail/health.json
                /*const monero_node_list = (Wallet.getWalletType() == 1) ? Backend.getNodeList("wownero") : Backend.getNodeList("monero")
                return monero_node_list*/
                /*if (Wallet.getWalletType() == 1) {
                    WalletNodeProvider.setCoinName("wownero")
                    WalletNodeProvider.startUpdates()
                } else {
                    WalletNodeProvider.setCoinName("monero")
                    WalletNodeProvider.startUpdates()
                }*/
                ////return WalletNodeProvider.nodes
            //}
            enabled: !WalletNodeProvider.loadingNodes
            Connections {
                target: WalletNodeProvider
                function onNodesUpdated() { 
                    console.log("Nodes updated, count:", WalletNodeProvider.nodes.length)
                    // Save last selected node since it gets reset on startup for some reason
                    // Note: last_selected_node gets reset to "" when user closes app before nodes are even updated
                    let savedNode = WalletNodeProvider.lastSelectedNode; // or settingsDialog.lastSelectedNode
                    if(savedNode && savedNode.length > 0) {
                        for(let i = 0; i < WalletNodeProvider.nodes.length; ++i) {
                            let node = WalletNodeProvider.nodes[i];
                            let nodeAddress = (typeof node === "string") ? node : node.address;
                            if(nodeAddress === savedNode) {
                                listView.currentIndex = i;
                                listView.positionViewAtIndex(i, ListView.Contain);
                                // Do the actual saving to QSettings here
                                listView.currentItem.saveSelectedNode()
                                break;
                            }
                        }
                    }
                }
            }
            delegate: Item {
                width: listView.width
                height: 25

                Rectangle {
                    anchors.fill: parent
                    color: parent.ListView.isCurrentItem ? "#ff8b3d" : "transparent"
                    radius: 3
                }
                
                function saveSelectedNode() {
                    listView.currentIndex = index
                    listView.positionViewAtIndex(index, ListView.Contain)
                    settingsDialog.save()
                    console.log("Saved selected node:", nodeAddressLabel.text)
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        saveSelectedNode()
                    }
                }

                RowLayout {
                    id: delegateRow
                    anchors.fill: parent
                    property string selectedNode: delegateRow.parent.ListView.isCurrentItem ? nodeAddressLabel.text : ""
                    property bool selectedNodeStatus: delegateRow.parent.ListView.isCurrentItem ? nodeStatusLabel.status : false

                    Label {
                        id: nodeStatusLabel
                        text: (typeof modelData === "string") ? qsTr("\uf1ce") : (status ? qsTr("\uf14a") : qsTr(FontAwesome.squareXmark))//"✅" : "❌"
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
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"//delegateRow.parent.ListView.isCurrentItem ? "#471d00" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
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
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
