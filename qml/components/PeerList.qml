import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

import "." as NeroshopComponents

Item {
    id: root
    property string selectedPeer: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedPeer
    property bool selectedPeerStatus: (listView.currentItem == null) ? "" : listView.currentItem.children[2].selectedPeerStatus
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
                Layout.minimumWidth: 150
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
                width: listView.width
                height: 25

                Rectangle {
                    anchors.fill: parent
                    color: parent.ListView.isCurrentItem ? "#8071a8" : "transparent"
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
                    property string selectedPeer: delegateRow.parent.ListView.isCurrentItem ? peerAddressLabel.text : ""
                    property bool selectedPeerStatus: delegateRow.parent.ListView.isCurrentItem ? peerStatusIcon.status : false
                    
                    Label {
                        id: peerStatusIcon
                        text: (typeof modelData !== "object") ? qsTr("\uf1ce") : ((modelData.status == 2) ? qsTr("\uf14a") : qsTr("\uf00d"))
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
                        color: delegateRow.parent.ListView.isCurrentItem ? "#471d00" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000")
                        font.bold: delegateRow.parent.ListView.isCurrentItem ? true : false
                        elide: Label.ElideRight
                    }

                    Label {
                        id: peerStatusLabel
                        Layout.minimumWidth: statusTitle.width
                        Layout.maximumWidth: statusTitle.width
                        text: (typeof modelData !== "object") ? "- -" : ((modelData.status == 2) ? qsTr("Online") : qsTr("Offline"))//modelData.status_str
                        color: peerAddressLabel.color
                        font.bold: peerAddressLabel.font.bold
                        elide: Label.ElideRight
                    }
                    
                    Label {
                        id: peerDistanceLabel
                        Layout.minimumWidth: distanceTitle.width
                        Layout.maximumWidth: distanceTitle.width
                        text: (typeof modelData !== "object") ? "- -" : modelData.distance
                        color: peerAddressLabel.color
                        font.bold: peerAddressLabel.font.bold
                        elide: Label.ElideRight
                    }
                }
            }
        }
    }
}
