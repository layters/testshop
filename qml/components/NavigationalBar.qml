import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "." as NeroshopComponents // Hint

RowLayout {
    id: buttons_menu

    Button {
        id: wallet_button
        text: qsTr("Wallet")
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextUnderIcon//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/wallet.png"//neroshopResourceDir + "/wallet.png"
        icon.color: "#ffffff"

        /*contentItem: Text {  
            text: wallet_button.text
            color: "#ffffff"
        }   */
                        
        background: Rectangle {
            color: wallet_button.down ? "white" : "transparent"//NeroshopComponents.Style.moneroOrangeColor
            border.color: color
            radius: 5
        }   
              
        MouseArea {
            //id: _mouse_area
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                // Style 1
                parent.background.border.color = NeroshopComponents.Style.moneroOrangeColor
                parent.icon.color = NeroshopComponents.Style.moneroOrangeColor
                // Style 2
                //parent.background.color = NeroshopComponents.Style.moneroOrangeColor
            }
            onExited: {
                // Style 1
                parent.background.border.color = "transparent"
                parent.icon.color = "#ffffff"
                // Style 2
                //parent.background.color = "transparent"
            }
            //onClicked: {}
        }                
    }
                            
    Button {
        text: qsTr("Seller Hub")
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/shop.png"//neroshopResourceDir + "/shop.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "royalblue"//"#808080"
        }                  
    }
    
    Button {
        text: qsTr("Messages")// todo: replace text with message_count
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/mail.png"//neroshopResourceDir + "/mail.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#524656"
        }            
    }
    
    Button {
        id: order_button
        text: qsTr("Orders")
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: "file:///" + neroshopResourcesDir + "/order.png"//neroshopResourceDir + "/order.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#607848"
        }                    
        
        NeroshopComponents.Hint {
            id: order_button_hint
            text: "Orders"
        }
    }      

    Button {
        id: account_button
        text: qsTr("User")//"Account Settings"
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        //flat: true
        //highlighted: true
        
        icon.source: "file:///" + neroshopResourcesDir + "/user.png"//neroshopResourceDir + "/user.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#cd8500"
        }                  
    }

    Button {
        id: cart_button
        // reference: https://doc.qt.io/qt-5/qml-qtquick-layouts-layout.html
        // fix alignment to StackLayout parent (no longer needed since we've set the preffered size, I think?)
        Layout.alignment: Qt.AlignTop
        // tell the layout that this child will have unique dimensions from the other children
        Layout.preferredHeight : 40
        Layout.preferredWidth : 100
     
        background: Rectangle {
            //width: cart_button.width; height: cart_button.height//width: 100; height: 40
            color: "#323232"
        }        

        Text {
            id: cart_button_text
            text: "0"
            color: "#ffffff"
            font.bold: true
            anchors.left: cart_button.background.left
            anchors.leftMargin: 20
            anchors.top: cart_button.background.top
            anchors.topMargin: (cart_button.background.height - this.height) / 2
        }
                
        Image {
            source: "file:///" + neroshopResourcesDir + "/cart.png"//neroshopResourceDir + "/cart.png"
            height: 24; width: 24
            anchors.left: cart_button_text.right
            anchors.leftMargin: 10
            anchors.top: cart_button.background.top
            anchors.topMargin: (cart_button.background.height - this.height) / 2
        }
    }
}
