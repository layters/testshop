import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import FontAwesome 1.0

// Wallet and Wallet Daemon settings page
import "../../components" as NeroshopComponents

Page {
    id: walletSettingsPage
    background: Rectangle {
        color: "transparent"//"#0a0a0a"
    }

    /*ScrollView {
                id: balanceTxScrollView
                anchors.fill: parent
                anchors.margins: 20        
                ScrollBar.vertical.policy: ScrollBar.AsNeeded//ScrollBar.AlwaysOn
                clip: true    
                contentWidth: parent.childrenRect.width//parent.width
                contentHeight: parent.childrenRect.height * 3*/
    
    ColumnLayout {
        id: balanceTxColumn
        anchors.margins: 20
        anchors.fill: parent
        spacing: 10
        //property real numberTextFontSize: 48//24
        property string textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
        property string baseColor: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#0e0e11" : "#101010") : "#f0f0f0"
        property string borderColor: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"
        property real radius: 15
                    
        // Balance
        Rectangle {
            id: balanceDisplay
            Layout.fillWidth: true//Layout.preferredWidth: parent.width - 200
            Layout.preferredHeight: 200
            radius: parent.radius//5
            color: balanceTxColumn.baseColor
            border.color: balanceTxColumn.borderColor
                        
            Column {
                anchors.centerIn: parent
                spacing: 0
                Row {
                    spacing: 5
                    TextField {
                        id: balanceUnlockedText
                        property double balance: !Wallet.opened ? 0.000000000000 : Wallet.balanceUnlocked//Wallet.getBalanceUnlocked()
                        text: balance.toFixed(12)
                        font.bold: true
                        font.pointSize: 48
                        color: balanceTxColumn.textColor
                        readOnly: true
                        selectByMouse: true
                        background: Rectangle {
                            color: "transparent"
                        }
                        padding: 0; leftPadding: 0; topPadding: 0 // With the padding at zero, we can freely set the Row spacing
                    }                                
                    Text {
                        id: balanceUnlockedCurrencySign
                        anchors.verticalCenter: parent.children[0].verticalCenter
                        text: qsTr("XMR")
                        font.bold: balanceUnlockedText.font.bold
                        font.pointSize: balanceUnlockedText.font.pointSize
                        color: "#404040"
                        visible: false
                    }
                }
                            
                Row {
                    anchors.horizontalCenter: parent.children[0].horizontalCenter
                    spacing: 5
                    Text {
                        anchors.verticalCenter: parent.children[1].verticalCenter
                        text: qsTr("\uf023 ")
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        font.pixelSize: 24
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    }
                    /*Image {
                        id: lockedIcon
                        anchors.verticalCenter: parent.children[1].verticalCenter
                        width: 32; height: width
                        source: "qrc:/images/lock.png"
                        mipmap: true
                    }*/
                    /*ColorOverlay {
                        anchors.fill: lockedIcon
                        source: lockedIcon
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        visible: lockedIcon.visible
                    }*/
                    TextField {
                        id: balanceLockedText
                        property double balance: !Wallet.opened ? 0.000000000000 : Wallet.balanceLocked//Wallet.getBalanceLocked()
                        text: balance.toFixed(12)
                        //font.bold: true
                        font.pointSize: 24
                        color: balanceTxColumn.textColor
                        readOnly: true
                        selectByMouse: true
                        background: Rectangle {
                            color: "transparent"
                        }
                        padding: 0; leftPadding: 0; topPadding: 0 // With the padding at zero, we can freely set the Row spacing
                    }
                    Text {
                        id: balanceLockedCurrencySign
                        anchors.verticalCenter: parent.children[1].verticalCenter
                        text: qsTr("XMR")
                        font.bold: balanceLockedText.font.bold
                        font.pointSize: balanceLockedText.font.pointSize
                        color: "#404040"
                        visible: balanceUnlockedCurrencySign.visible
                    } 
                }                               
            }
        }
                    
        NeroshopComponents.TabBar {
            id: tabBar
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop//; Layout.topMargin: 10
            model: ["Transactions", "Send", "Receive"]
            color0: NeroshopComponents.Style.moneroOrangeColor
            Component.onCompleted: {
                buttonAt(0).checked = true
            }
        }               
        
        StackLayout { // TODO: add send, receive sections or maybe just place everything in a single page instead of dividing them into stacklayout item sections
            id: walletStack
            Layout.fillWidth: true
            Layout.preferredHeight: 500//Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            // Transactions
            Item {
                id: txsDisplay
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true

                Column {               
                    width: parent.width; height: parent.height
                    spacing: 10
                    /*Text {
                        text: qsTr("Transactions")
                        color: balanceTxColumn.textColor
                        font.bold: true
                        font.pointSize: 16
                    }*/
                            
                    Flickable {
                        width: parent.width; height: 500
                        contentWidth: parent.width; contentHeight: (150 * txRepeater.count) + (txFlow.spacing * (txRepeater.count - 1))
                        clip: true
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AlwaysOff//AsNeeded
                        }
                        Column {//Flow {
                            id: txFlow
                            anchors.fill: parent//width: childrenRect.width * txRepeater.count; height: childrenRect.height * txRepeater.count//width: parent.contentWidth//; height: parent.height
                            spacing: 7
                            Repeater {
                                id: txRepeater
                                model: 10//3
                                delegate: Rectangle {
                                    color: balanceTxColumn.baseColor
                                    width: parent.width; height: 150
                                    radius: 5
                                }
                            }
                        }
                    }
                }
            }
            // Send
            Item {}
            // Receive
            Item {
                    Image {
                        source: "image://wallet_qr/%1".arg((!Wallet.opened) ? "" : Wallet.getPrimaryAddress())////"image://wallet_qr/%1".arg(Wallet.getPrimaryAddress())
                        sourceSize {
                            width: 200
                            height: 200
                        }
                    }
            }
        }    
    } // root Layout
}
