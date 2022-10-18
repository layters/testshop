import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12// replaced with "import Qt5Compat.GraphicalEffects 1.15" in Qt6// ColorOverlay

import "." as NeroshopComponents // Hint

RowLayout {
    id: navBar
    readonly property string defaultButtonColor: "#6b5b95"
    property bool useDefaultButtonColor: false
    function getCheckedButton() {
        return navBarButtonGroup.checkedButton;
    }
    function uncheckAllButtons() {
        navBarButtonGroup.checkState = Qt.Unchecked;
    }
        
    ButtonGroup {
        id: navBarButtonGroup
        exclusive: true
        //checkState: conditionParentBox.checkState
        onClicked: {
            console.log("Selected", button.text, "button")
            button.checked = true
            if(button.text == walletButton.text) {
                pageLoader.source = "../pages/settings/SettingsWallet.qml"//_stackview.currentIndex = 0
            }
            if(button.text == sellersHubButton.text) {
                pageLoader.source = "../pages/settings/SettingsSellersHub.qml"
            }
            if(button.text == messagesButton.text) {
                pageLoader.source = "../pages/settings/SettingsMessages.qml"
            }
            if(button.text == ordersButton.text) {
                pageLoader.source = "../pages/settings/SettingsOrders.qml"
            }
            if(button.text == accountSettingsButton.text) {
                pageLoader.source = "../pages/settings/SettingsAccount.qml"
            }                                                        
        }
    }
        
    Button {
        id: walletButton
        text: qsTr("Wallet")
        ButtonGroup.group: navBarButtonGroup // attaches a button to a button group
        display: AbstractButton.IconOnly
        //checkable: true
        
        icon.source: "file:///" + neroshopResourcesDir + "/wallet.png"
        icon.color: (!checked && this.hovered) ? NeroshopComponents.Style.moneroOrangeColor : "#ffffff"
        //property string reservedColor: NeroshopComponents.Style.moneroOrangeColor
                        
        background: Rectangle {
            color: (parent.checked) ? NeroshopComponents.Style.moneroOrangeColor : "transparent"
            border.color: NeroshopComponents.Style.moneroOrangeColor
            border.width: (!parent.checked && parent.hovered) ? 1 : 0
            radius: 5
        }
           
        NeroshopComponents.Hint {
            id: walletButtonHint
            visible: parent.hovered
            text: parent.text
            pointer.visible: false
            delay: 0 // Show immediately on hovering over button
            //textObject.font.bold: true
        }
    }
                            
    Button {
        id: sellersHubButton
        text: qsTr("Sellers' Hub")
        ButtonGroup.group: navBarButtonGroup
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/shop.png"
        icon.color: (!checked && this.hovered) ? reservedColor : "#ffffff"
        property string reservedColor: (useDefaultButtonColor) ? defaultButtonColor : "royalblue"
                        
        background: Rectangle {
            color: (parent.checked) ? parent.reservedColor : "transparent"
            border.color: parent.reservedColor
            border.width: (!parent.checked && parent.hovered) ? 1 : 0
            radius: 5
        }
           
        NeroshopComponents.Hint {
            visible: parent.hovered
            text: parent.text
            pointer.visible: false
            delay: 0 // Show immediately on hovering over button
            //textObject.font.bold: true
        }
    }
    
    Button {
        id: messagesButton
        text: (messagesButton.dummy_count > 0) ? qsTr("Messages : %1").arg(dummy_count.toString()) : qsTr("Messages")
        ButtonGroup.group: navBarButtonGroup
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        property int dummy_count: 0
        
        icon.source: "file:///" + neroshopResourcesDir + "/mail.png"
        icon.color: (!checked && this.hovered) ? reservedColor : "#ffffff"
        property string reservedColor: (useDefaultButtonColor) ? defaultButtonColor : "#524656"
                        
        background: Rectangle {
            color: (parent.checked) ? parent.reservedColor : "transparent"
            border.color: parent.reservedColor
            border.width: (!parent.checked && parent.hovered) ? 1 : 0
            radius: 5
        }
           
        NeroshopComponents.Hint {
            visible: parent.hovered
            text: parent.text
            pointer.visible: false
            delay: 0
            //textObject.font.bold: true
        }
    }
    
    Button {
        id: ordersButton
        text: qsTr("Orders")
        ButtonGroup.group: navBarButtonGroup
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/order.png"
        icon.color: (!checked && this.hovered) ? reservedColor : "#ffffff"
        property string reservedColor: (useDefaultButtonColor) ? defaultButtonColor : "#607848"
                        
        background: Rectangle {
            color: (parent.checked) ? parent.reservedColor : "transparent"
            border.color: parent.reservedColor
            border.width: (!parent.checked && parent.hovered) ? 1 : 0
            radius: 5
        }
           
        NeroshopComponents.Hint {
            visible: parent.hovered
            text: parent.text
            pointer.visible: false
            delay: 0
            //textObject.font.bold: true
        }
    }      

    Button {
        id: accountSettingsButton
        text: qsTr("Account Settings")//qsTr("User")
        ButtonGroup.group: navBarButtonGroup
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/user.png"
        icon.color: (!checked && this.hovered) ? reservedColor : "#ffffff"
        property string reservedColor: (useDefaultButtonColor) ? defaultButtonColor : "#cd8500"
                        
        background: Rectangle {
            color: (parent.checked) ? parent.reservedColor : "transparent"
            border.color: parent.reservedColor
            border.width: (!parent.checked && parent.hovered) ? 1 : 0
            radius: 5
        }
           
        NeroshopComponents.Hint {
            visible: parent.hovered
            text: parent.text
            pointer.visible: false
            delay: 0
            //textObject.font.bold: true
        }
    }

    Button {
        id: cartButton
        ////ButtonGroup.group: navBarButtonGroup
        ////autoExclusive: true
        // reference: https://doc.qt.io/qt-5/qml-qtquick-layouts-layout.html
        Layout.alignment: Qt.AlignTop
        // tell the layout that this child will have unique dimensions from the other children
        Layout.preferredHeight : 40
        Layout.preferredWidth : 100
        property string reservedColor: "#323232"//(useDefaultButtonColor) ? defaultButtonColor : "#323232"
     
        background: Rectangle {
            //width: cartButton.width; height: cartButton.height//width: 100; height: 40
            color: parent.reservedColor//(parent.checked) ? parent.reservedColor : "transparent"
            //border.color: parent.reservedColor
            //border.width: (!parent.checked && parent.hovered) ? 1 : 0
            radius: 5
        }        

        Text {
            id: cartButtonText
            text: "0"
            color: "#ffffff"
            font.bold: true
            anchors.left: cartButton.background.left
            anchors.leftMargin: 20
            anchors.top: cartButton.background.top
            anchors.topMargin: (cartButton.background.height - this.height) / 2
        }
                
        Image {
            id: cartButtonIcon
            source: "file:///" + neroshopResourcesDir + "/cart.png"
            height: 24; width: 24
            anchors.left: cartButtonText.right
            anchors.leftMargin: 10
            anchors.top: cartButton.background.top
            anchors.topMargin: (cartButton.background.height - this.height) / 2
        }
        /*ColorOverlay {
            anchors.fill: cartButtonIcon
            source: cartButtonIcon
            color: "#ffffff"//(!parent.checked && parent.hovered) ? parent.reservedColor : "#ffffff"
            visible: cartButtonIcon.visible
        }*/
           
        NeroshopComponents.Hint {
            visible: parent.hovered
            text: "Cart"
            pointer.visible: false
            delay: 0
            //textObject.font.bold: true
        }        
        
        onClicked: {
            navBar.uncheckAllButtons();
            pageLoader.source = "../pages/CartPage.qml"
        }
    }
}