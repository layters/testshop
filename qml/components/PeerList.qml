import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

import "." as NeroshopComponents

Item {
    id: root
    property string selectedPeer: {
        if (selectedIndex < 0 || selectedIndex >= listView.count) return "";
        let item = listView.itemAtIndex(selectedIndex);
        if (item === null) return "";
        return item.children[2].selectedPeer;
    }
    property int selectedPeerStatus: {
        if (selectedIndex < 0 || selectedIndex >= listView.count) return 0;
        let item = listView.itemAtIndex(selectedIndex);
        if (item === null) return 0;
        return item.children[2].selectedPeerStatus;
    }
    property int selectedIndex: -1
    property alias currentIndex: listView.currentIndex
    property alias model: listView.model
    property alias count: listView.count
    property alias list: listView
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            spacing: 2
            property real titleBoxRadius: 3
            property real titleBoxMinHeight: 25

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: parent.titleBoxMinHeight
                color: "#6c6c6f"
                radius: parent.titleBoxRadius

                Label {
                    text: qsTr("Connected Peer")
                    color: "white"
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            Rectangle {
                id: statusTitle
                visible: false
                Layout.minimumWidth: 100
                Layout.minimumHeight: parent.titleBoxMinHeight
                color: "#6c6c6f"
                radius: parent.titleBoxRadius

                Label {
                    text: qsTr("Status")
                    color: "white"
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                }
            }
            
            Rectangle {
                id: distanceTitle
                Layout.minimumWidth: 100
                Layout.minimumHeight: parent.titleBoxMinHeight
                color: "#6c6c6f"
                radius: parent.titleBoxRadius

                Label {
                    text: qsTr("Distance")
                    color: "white"
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        ListView {
            id: listView
            Component.onCompleted: { 
                currentIndex = -1
            }//currentIndex: -1
            onCurrentIndexChanged: {}
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical: ScrollBar { }
            model: {
                if(networkMonitor.networkStatus == null) return [];
                if(networkMonitor.networkStatus.hasOwnProperty("peers")) {
                    return networkMonitor.networkStatus.peers
                }
                return []
            }
            delegate: Item {
                id: listItem
                width: listView.width
                height: 25
                property bool hovered: false

                Rectangle {
                    anchors.fill: parent
                    color: (index === root.selectedIndex) ? "#8071a8" : "transparent"
                    radius: 3
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.hovered = true
                    onExited: parent.hovered = false
                    onClicked: {
                        root.selectedIndex = index
                        // synchronize: Update listView.currentIndex whenever selectedIndex changes (or we can ignore currentIndex entirely)
                        if (listView.currentIndex !== root.selectedIndex) {
                            listView.currentIndex = root.selectedIndex;
                            listView.positionViewAtIndex(root.selectedIndex, ListView.Contain)
                        }
                    }
                }

                RowLayout {
                    id: delegateRow
                    anchors.fill: parent
                    property string selectedPeer: (index === root.selectedIndex) ? peerAddressLabel.text : ""
                    property int selectedPeerStatus: (index === root.selectedIndex) ? peerStatusIcon.status : 0
                    
                    Label {
                        id: peerStatusIcon
                        text: (typeof modelData !== "object") ? qsTr("\uf1ce") : ((modelData.status == 2) ? qsTr("\uf14a") : qsTr(FontAwesome.squareXmark))
                        color: (typeof modelData !== "object") ? "royalblue" : ((status == 2) ? "#698b22" : "#dd4b4b")
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        Layout.maximumWidth: 25
                        property int status: (typeof modelData !== "object") ? 0 : modelData.status
                    }

                    Label {
                        id: peerAddressLabel
                        Layout.fillWidth: true
                        text: (typeof modelData !== "object") ? "" : modelData.address//(modelData.address + ":" + modelData.port.toString())
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"//(index === root.selectedIndex) ? "#471d00" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
                        font.bold: (index === root.selectedIndex) ? true : false
                        elide: Label.ElideRight
                        
                        function isElided() {
                            // Compare full text width with available width (peerAddressLabel.width)
                            return metrics.width > peerAddressLabel.width + 1 // Add 1px tolerance
                        }
                        
                        NeroshopComponents.Hint {
                            visible: listItem.hovered && parent.isElided()
                            height: contentHeight + 20; width: contentWidth + 20
                            text: qsTr(peerAddressLabel.text)
                            pointer.visible: false
                        }
                    }
                    
                    TextMetrics { // metrics.width is the full width of the text without eliding
                        id: metrics
                        font: peerAddressLabel.font
                        text: peerAddressLabel.text
                    }

                    Label {
                        id: peerStatusLabel
                        visible: statusTitle.visible
                        Layout.minimumWidth: statusTitle.width
                        Layout.maximumWidth: statusTitle.width
                        text: (typeof modelData !== "object") ? "- -" : ((modelData.status == 2) ? qsTr("Online") : qsTr("Offline"))//modelData.status_str
                        color: peerAddressLabel.color
                        font.bold: peerAddressLabel.font.bold
                        elide: Label.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    Label {
                        id: peerDistanceLabel
                        Layout.minimumWidth: distanceTitle.width
                        Layout.maximumWidth: distanceTitle.width
                        text: (typeof modelData !== "object") ? "- -" : modelData.distance
                        color: peerAddressLabel.color
                        font.bold: peerAddressLabel.font.bold
                        elide: Label.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
