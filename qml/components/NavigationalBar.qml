import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "." as NeroshopComponents // Hint

RowLayout {
    id: buttons_menu
    anchors.left: parent.right
    anchors.leftMargin: (-this.width - 20)
    anchors.top: parent.top
    anchors.topMargin: 20
        
    Button {
        text: qsTr("Seller Hub")
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: neroshopResourceDir + "/shop.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "royalblue"//"#808080"
        }                  
    }
    
    Button {
        text: qsTr("Messages")// todo: replace with message_count
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: neroshopResourceDir + "/mail.png"
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
        
        icon.source: neroshopResourceDir + "/order.png"
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
        
        icon.source: neroshopResourceDir + "/user.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#cd8500"
        }                  
    }

    Button {
        id: cart_button
        // ref: https://doc.qt.io/qt-5/qml-qtquick-layouts-layout.html
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
            source: neroshopResourceDir + "/cart.png"
            height: 24; width: 24
            anchors.left: cart_button_text.right
            anchors.leftMargin: 10
            anchors.top: cart_button.background.top
            anchors.topMargin: (cart_button.background.height - this.height) / 2
        }
    }
    
    /*Button {
        text: qsTr("")
        //onClicked: _stackview.currentIndex = 0
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        
        icon.source: neroshopResourceDir + "/image.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: "#808080"
        }                    
    }*/                  
}
