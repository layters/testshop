import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import FontAwesome 1.0
import neroshop.CurrencyExchangeRates 1.0

// Wallet page
import "../../components" as NeroshopComponents

Page {
    id: walletSettingsPage
    background: Rectangle {
        color: "transparent"//"#0a0a0a"
    }
    
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
                        visible: (settingsDialog.balanceDisplay == 1) ? false : true
                        property double balance: !Wallet.opened ? 0.000000000000 : Wallet.balanceUnlocked
                        text: balance.toFixed(settingsDialog.balanceAmountPrecision)
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
                        visible: settingsDialog.showCurrencySign && balanceUnlockedText.visible
                    }
                }
                            
                Row {
                    anchors.horizontalCenter: !balanceUnlockedText.visible ? parent.horizontalCenter: parent.children[0].horizontalCenter
                    spacing: 5
                    Text {
                        visible: balanceLockedText.visible
                        anchors.verticalCenter: parent.children[1].verticalCenter
                        text: qsTr("\uf023 ")
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        font.pixelSize: balanceLockedText.font.pointSize
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    }
                    /*Image {
                        id: lockedIcon
                        anchors.verticalCenter: parent.children[1].verticalCenter
                        width: 32; height: width
                        source: "qrc:/assets/images/lock.png"
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
                        visible: (settingsDialog.balanceDisplay == 2) ? false : true
                        property double balance: !Wallet.opened || (Wallet.balanceLocked == Wallet.balanceUnlocked) ? 0.000000000000 : Wallet.balanceLocked - Wallet.balanceUnlocked
                        text: balance.toFixed(settingsDialog.balanceAmountPrecision)
                        font.bold: !balanceUnlockedText.visible ? true : false
                        font.pointSize: !balanceUnlockedText.visible ? 48 : 24
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
                        visible: settingsDialog.showCurrencySign && balanceLockedText.visible
                    } 
                }                               
            }
        }
                    
        NeroshopComponents.TabBar {
            id: tabBar
            Layout.alignment: Qt.AlignHCenter
            model: ["Transactions", "Send", "Receive"/*, "\uf013"*/]
            color0: NeroshopComponents.Style.moneroOrangeColor
            Component.onCompleted: {
                buttonAt(0).checked = true
                /*buttonAt(3).width = 50
                buttonAt(3).contentItem.font.bold = true
                buttonAt(3).contentItem.font.family = FontAwesome.fontFamily*/
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
                    contentWidth: parent.width; contentHeight: (75 * txList.count) + (txList.spacing * (txList.count - 1))
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded//AlwaysOff
                    }
                    ListView {
                        id: txList
                        anchors.fill: parent
                        //width: parent.contentWidth/*parent.width*/; height: parent.contentHeight//childrenRect.height * txRepeater.count//width: parent.contentWidth//; height: parent.height
                        //anchors.centerIn: parent//anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 7
                        Component.onCompleted: console.log("tx count", /*txRepeater.*/count)
                        model: Wallet.transfers
                        delegate: Item {
                            width: parent.width
                            height: txDetailsRect.visible ? (txRect.height + txDetailsRect.height) : txRect.height
                            Column {
                                anchors.fill: parent
                                Rectangle {
                                    id: txRect
                                    color: balanceTxColumn.baseColor
                                    width: parent.width; height: 75
                                    radius: 5
                                    Rectangle {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left; anchors.leftMargin: 5
                                        width: parent.height - 10; height: width
                                        color: "transparent"
                                        border.color: balanceTxColumn.borderColor
                                    
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.is_incoming ? "\uf063" : "\uf062"
                                            color: (!Wallet.opened) ? "#ffffff" : (modelData.is_incoming ? "#2cba78" : "#c32235")
                                            font.bold: true; font.family: FontAwesome.fontFamily
                                            font.pixelSize: 32
                                        }
                                    }
                                    Column {
                                        anchors.left: parent.children[0].right; anchors.leftMargin: 20
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 1
                                        Text {
                                            text: modelData.is_incoming ? qsTr("Received") : qsTr("Sent")
                                            color: balanceTxColumn.textColor
                                        }
                                        Text {
                                            text: "2024-01-01 00:00" // TODO: get actual tx date and time
                                            color: "#777"
                                            font.pointSize: 10
                                            visible: false // <- hide this for now
                                        }
                                    }
                                    Column {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                        Text {
                                            text: (!Wallet.opened) ? "" : qsTr("%1 %2").arg(modelData.is_incoming ? "+" : "-").arg(modelData.amount.toFixed(12))
                                            color: (!Wallet.opened) ? "#ffffff" : (modelData.is_incoming ? "#2cba78" : "#c32235")
                                            font.bold: true
                                        }
                                        Text {
                                            text: (!Wallet.opened) ? "" : qsTr("%1 %2%3 %4").arg(modelData.is_incoming ? "+" : "-").arg(Backend.getCurrencySign(priceDisplayText.currency)).arg(CurrencyExchangeRates.getXmrPrice(priceDisplayText.currency) * modelData.amount.toFixed(12)).arg(priceDisplayText.currency.toUpperCase())
                                            color: "#777"
                                            font.pointSize: 10
                                            visible: (Wallet.getWalletType() == 0 && priceDisplayText.currency != "XMR")
                                        }
                                    }
                                    Row {
                                        anchors.right: parent.right; anchors.rightMargin: 20
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 10
                                        Button {
                                            width: contentItem.contentWidth + 20; height: contentItem.contentHeight + 20
                                            text: qsTr("Open in browser")
                                            display: AbstractButton.IconOnly
                                            icon.source: "qrc:/assets/images/external_link.png"
                                            icon.color: balanceTxColumn.textColor
                                            hoverEnabled: true
                                            background: Rectangle {
                                                color: "transparent"
                                                border.color: parent.hovered ? balanceTxColumn.borderColor : color
                                                radius: 3
                                            }
                                            onClicked: {
                                                let network_type = Wallet.getNetworkTypeString()
                                                if(network_type != "mainnet") {
                                                    Qt.openUrlExternally("https://" + network_type + "." + settingsDialog.blockExplorer)
                                                    return
                                                }
                                                Qt.openUrlExternally("https://" + settingsDialog.blockExplorer)
                                            }
                                            MouseArea { 
                                                anchors.fill: parent
                                                onPressed: mouse.accepted = false // without this, Button.onClicked won't work
                                                cursorShape: Qt.PointingHandCursor
                                            }
                                            NeroshopComponents.Hint {
                                                visible: parent.hovered
                                                height: contentHeight + 20; width: contentWidth + 20
                                                text: parent.text
                                                pointer.visible: false
                                                timeout: 1000; delay: 0
                                            }
                                        }
                                        // Show more button
                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: txDetailsRect.visible ? qsTr(FontAwesome.chevronUp) : qsTr(FontAwesome.chevronDown)
                                            color: balanceTxColumn.textColor
                                            font.bold: true
                                            font.family: FontAwesome.fontFamily
                                            font.pointSize: 16
                                            property bool hovered: false
                                            MouseArea {
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                onEntered: parent.hovered = true
                                                onExited: parent.hovered = false
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    if(!txDetailsRect.visible) {
                                                        txDetailsRect.visible = true
                                                        return
                                                    }
                                                    if(txDetailsRect.visible) {
                                                        txDetailsRect.visible = false
                                                        return
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                // TX details rect
                                Rectangle {
                                    id: txDetailsRect
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    width: parent.width - 10
                                    height: 25
                                    color: balanceTxColumn.baseColor
                                    visible: false
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Additional Transaction Details"
                                        color: balanceTxColumn.textColor
                                    }
                                }
                            } // Column
                        } // Item delegate
                    }
                }
            }
            // Send
            Item {
                id: sendTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                Flickable {
                    anchors.fill: parent
                    contentWidth: width; contentHeight: sendTabColumn.childrenRect.height
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    Column {
                        id: sendTabColumn
                        anchors.fill: parent
                        spacing: 20 // spacing between each item inside the column
                        ListModel {
                            id: recipientModel
                            ListElement {}
                        }
                        Repeater {
                            id: recipientRepeater
                            model: recipientModel
                            delegate: Row {
                                anchors.horizontalCenter: parent.horizontalCenter
                                spacing: 5
                                // addressField
                                TextField {
                                    id: addressField
                                    width: 450/*parent.width*/; height: 50
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
                                    rightPadding: resolveButton.visible ? (15 + resolveButton.width) : leftPadding
                                    Button {
                                        id: resolveButton
                                        text: qsTr("Resolve")
                                        anchors.right: parent.right
                                        anchors.rightMargin: 10
                                        anchors.verticalCenter: parent.verticalCenter
                                        hoverEnabled: true
                                        visible: Wallet.isValidOpenAliasAddress(addressField.text)
                                        ////onClicked: // TODO: handle OpenAlias resolution 
                                        background: Rectangle {
                                            color: NeroshopComponents.Style.moneroGrayColor
                                            radius: 10
                                        }
                                        contentItem: Text {
                                            text: parent.text
                                            color: "#ffffff"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onPressed: mouse.accepted = false
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                    }
                                }
                                // amountField
                                TextField {
                                    id: amountField
                                    width: 200/*parent.width*/; height: addressField.height
                                    placeholderText: qsTr("Amount")//0.000000000000
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
                                        MouseArea {
                                            anchors.fill: parent
                                            onPressed: mouse.accepted = false
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                    }
                                }
                                //removeRecipientButton
                                Button {
                                    id: removeRecipientButton
                                    width: 20; height: 20
                                    anchors.verticalCenter: amountField.verticalCenter
                                    text: qsTr(FontAwesome.xmark)
                                    hoverEnabled: true
                                    visible: (recipientRepeater.count > 1)
                                    
                                    background: Rectangle {
                                        color: "transparent"//parent.hovered ? "firebrick" : "transparent"
                                        radius: 5
                                        opacity: 0.7
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        font.bold: true
                                        font.family: FontAwesome.fontFamily
                                    }
                                    onClicked: {
                                        recipientModel.remove(index)
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onPressed: mouse.accepted = false
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                            } // Row
                        } // Repeater
                        // addRecipientButton
                        Button {
                            id: addRecipientButton
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("+ Add Recipient")
                            width: 140; height: contentItem.content + 30
                            background: Rectangle {
                                color: parent.hovered ? "#698b22" : "#506a1a"
                                radius: 5
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                recipientModel.append({})
                            }
                            MouseArea {
                                anchors.fill: parent
                                onPressed: mouse.accepted = false
                                cursorShape: Qt.PointingHandCursor
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
                                if(!Wallet.isSynced()) {
                                    messageBox.text = qsTr("wallet must be fully synced with daemon")
                                    messageBox.open()
                                    return
                                }
                                if(settingsDialog.requirePasswordOnWithdrawal) {
                                    walletPasswordSendPrompt.open()
                                    walletPasswordSendPrompt.editAt(1).forceActiveFocus()
                                }
                                else Wallet.transfer(recipientRepeater.itemAt(0).children[0].text, recipientRepeater.itemAt(0).children[1].text)
                            }
                            MouseArea {
                                anchors.fill: parent
                                onPressed: mouse.accepted = false
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }
                // Enter wallet password message prompt
                NeroshopComponents.MessageBox {
                    id: walletPasswordSendPrompt
                    x: mainWindow.x + (mainWindow.width - this.width) / 2
                    y: mainWindow.y + (mainWindow.height - this.height) / 2
                    title: qsTr("Send")
                    text: qsTr("Please enter your wallet password")
                    buttonModel: ["Cancel", "OK"]
                    editModel: 2//1
                    buttonRow.state: "centered"; buttonRow.width: editColumn.width
                    editColumn.anchors.topMargin: 0
                    Component.onCompleted: {
                        //textObject.horizontalAlignment = TextEdit.AlignLeft
                        
                        const error_edit = editAt(0)
                        error_edit.readOnly = true
                        error_edit.background.color = "transparent"
                        const password_edit = editAt(1)
                        password_edit.echoMode = TextInput.Password
                        password_edit.inputMethodHints = Qt.ImhSensitiveData            
                        buttonAt(0).color = NeroshopComponents.Style.moneroGrayColor
                        buttonAt(1).color = "#66578e"
                        
                        onCloseCallback = function() { 
                            error_edit.color = "#353637"
                            error_edit.text = ""
                            password_edit.text = ""
                        }
                        
                        buttonAt(0).onClickedCallback = function() { 
                            onCloseCallback()
                            close()
                        }
                        buttonAt(1).onClickedCallback = function() {
                            if(!Wallet.verifyPassword(password_edit.text)) {
                                error_edit.color = "#b22222"
                                error_edit.text = qsTr("Password is incorrect. Try again")
                                password_edit.text = ""
                                return;
                            }
                            if(recipientRepeater.count == 1) {
                                Wallet.transfer(recipientRepeater.itemAt(0).children[0].text, recipientRepeater.itemAt(0).children[1].text)
                            } else {
                                let recipients = [];
                                
                                for (let i = 0; i < recipientRepeater.count; i++) {
                                    let address = recipientRepeater.itemAt(i).children[0].text;
                                    let amount = recipientRepeater.itemAt(i).children[1].text;
                                    
                                    // Create an object with address and amount fields
                                    let recipientObject = {
                                        address: address,
                                        amount: amount
                                    };

                                    // Push the object into the recipients array
                                    recipients.push(recipientObject);
                                }
                                
                                Wallet.transfer(recipients)
                            }
                            // TODO: clear address and amount field if transfer was successful
                            onCloseCallback()
                            close()
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
                            TextArea {
                                id: primaryAddressField
                                width: 750; height: 50
                                text: !Wallet.opened ? "" : Wallet.getPrimaryAddress()
                                color: balanceTxColumn.textColor
                                readOnly: true
                                selectByMouse: true
                                wrapMode: Text.Wrap
                                background: Rectangle { 
                                    color: balanceTxColumn.baseColor
                                    border.color: balanceTxColumn.borderColor
                                    border.width: parent.activeFocus ? 2 : 1
                                    radius: 5//balanceTxColumn.radius
                                }
                                rightPadding: leftPadding
                                verticalAlignment: TextEdit.AlignVCenter // align the text within the center of TextArea item's height
                                horizontalAlignment: TextEdit.AlignHCenter
                            }
                            Button {
                                id: copyButton
                                width: 50; height: primaryAddressField.height
                                text: qsTr("Copy")
                                display: AbstractButton.IconOnly
                                icon.source: "qrc:/assets/images/copy.png"
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
                        // TODO: add subaddress list
                    }
                }
            }
        }    
    } // root Layout
}
