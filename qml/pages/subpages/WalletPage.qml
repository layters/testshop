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
        spacing: 30
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
                        property double balance: !Wallet.opened ? 0.000000000000 : Wallet.balanceUnlocked
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
                        property double balance: !Wallet.opened ? 0.000000000000 : Wallet.balanceLocked
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
            Layout.alignment: Qt.AlignHCenter
            model: ["Transactions", "Send", "Receive"]
            color0: NeroshopComponents.Style.moneroOrangeColor
            Component.onCompleted: {
                buttonAt(0).checked = true
            }
        }               
        
        StackLayout { 
            id: walletStack
            Layout.fillWidth: true
            Layout.preferredHeight: 300//500//Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            // Transactions
            Item {
                id: txsDisplay
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                            
                Flickable {
                    anchors.fill: parent
                    contentWidth: parent.width/*950*/; contentHeight: (75 * txRepeater.count) + (txFlow.spacing * (txRepeater.count - 1))
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOff//AsNeeded
                    }
                    Column {//Flow {
                        id: txFlow
                        anchors.fill: parent//
                        //width: parent.contentWidth/*parent.width*/; height: parent.contentHeight//childrenRect.height * txRepeater.count//width: parent.contentWidth//; height: parent.height
                        //anchors.centerIn: parent//anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 7
                        Repeater {
                            id: txRepeater
                            model: 10//3
                            delegate: Rectangle {
                                color: balanceTxColumn.baseColor
                                width: /*950*/parent.width; height: 75//150
                                radius: 5
                            }
                        }
                    }
                }
            }
            // Send
            Item {
                id: sendTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                Column {
                    anchors.fill: parent
                    spacing: 20 // spacing between each item inside the column
                    // addressField
                    TextField {
                        id: addressField
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 500/*parent.width*/; height: 50
                        placeholderText: qsTr("Receiver address")
                        color: balanceTxColumn.textColor
                        selectByMouse: true
                        maximumLength: 95//200 // TODO: set to 200 when Seraphis goes live
                        background: Rectangle { 
                            color: balanceTxColumn.baseColor
                            border.color: balanceTxColumn.borderColor
                            border.width: parent.activeFocus ? 2 : 1
                            radius: balanceTxColumn.radius
                        }
                    }
                    // amountField
                    TextField {
                        id: amountField
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 500/*parent.width*/; height: 50
                        placeholderText: qsTr("Amount")
                        color: balanceTxColumn.textColor
                        selectByMouse: true
                        validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + Number(12) + "})?$") }
                        background: Rectangle { 
                            color: balanceTxColumn.baseColor
                            border.color: balanceTxColumn.borderColor
                            border.width: parent.activeFocus ? 2 : 1
                            radius: balanceTxColumn.radius
                        }
                        rightPadding: 15 + allButton.width
                        Button {
                            id: allButton
                            text: qsTr("\uf534")
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            implicitWidth: 32; implicitHeight: 24
                            hoverEnabled: true
                            onClicked: amountField.text = Wallet.getBalanceUnlocked().toFixed(12)
                            background: Rectangle {
                                color: NeroshopComponents.Style.moneroGrayColor
                                radius: 10//amountField.background.radius//5
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.bold: true
                                font.family: FontAwesome.fontFamily
                            }
                        }
                    }
                    // sendButton
                    Button {
                        id: sendButton
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 500; height: contentItem.contentHeight + 30
                        text: qsTr("Transfer")
                            background: Rectangle {
                                color: parent.hovered ? "#ff7214" : NeroshopComponents.Style.moneroOrangeColor
                                radius: balanceTxColumn.radius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                Wallet.transfer(addressField.text, amountField.text)
                            }
                    }
                }
            }
            // Receive
            Item {
                id: receiveTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                Flickable {
                    anchors.fill: parent
                    contentWidth: width; contentHeight: receiveTabColumn.childrenRect.height
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    Column {
                        id: receiveTabColumn
                        anchors.fill: parent
                        spacing: 20
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 5
                            // Primary address
                            TextField {
                                id: primaryAddressField
                                /*width: contentWidth; */height: 50
                                text: !Wallet.opened ? "" : Wallet.getPrimaryAddress()
                                color: balanceTxColumn.textColor
                                readOnly: true
                                selectByMouse: true
                                background: Rectangle { 
                                    color: balanceTxColumn.baseColor
                                    border.color: balanceTxColumn.borderColor
                                    border.width: parent.activeFocus ? 2 : 1
                                    radius: balanceTxColumn.radius
                                }
                                rightPadding: leftPadding
                            }
                            Button {
                                id: copyButton
                                width: 50; height: primaryAddressField.height
                                text: qsTr("Copy")
                                display: AbstractButton.IconOnly
                                icon.source: "qrc:/images/copy.png"
                                icon.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                background: Rectangle {
                                    color: "transparent"//"#808080"
                                    border.color: parent.hovered ? balanceTxColumn.borderColor : "transparent"
                                    radius: 15
                                }
                                onClicked: {
                                    primaryAddressField.selectAll()
                                    primaryAddressField.copy()////Backend.copyTextToClipboard(primaryAddressField.text)
                                }
                            }
                        }
                        // Wallet QR
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: imageSize + 50; height: this.width
                            radius: 5
                            property real imageSize: 200
                            Image {
                                anchors.centerIn: parent
                                source: "image://wallet_qr/%1".arg(!Wallet.opened ? "" : Wallet.getPrimaryAddress())////"image://wallet_qr/%1".arg(Wallet.getPrimaryAddress())
                                sourceSize {
                                    width: parent.imageSize
                                    height: parent.imageSize
                                }
                            }
                        }
                    }
                }
            }
        }    
    } // root Layout
}
