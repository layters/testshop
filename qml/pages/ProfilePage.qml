// This page represents the user's profile where all the user's listings, contact information, etc. will be displayed
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12 // ColorOverlay

import FontAwesome 1.0

import neroshop.CurrencyExchangeRates 1.0

import "../components" as NeroshopComponents

Page {
    id: profilePage
    background: Rectangle {
        color: "transparent"
    }
    property var userModel: Backend.getUser((productModel === null) ? messagesModel.sender_id : productModel.seller_id)
    property var productModel: null // <- the product listing that redirected user to this profile page
    property var ratingsModel: Backend.getSellerRatings((productModel === null) ? messagesModel.sender_id : productModel.seller_id)
    property var listingsModel: Backend.getInventory((productModel === null) ? messagesModel.sender_id : productModel.seller_id, settingsDialog.hideIllegalProducts)//Backend.getListingsBySearchTerm(productModel.seller_id)
    property var messagesModel: null
    
    ColumnLayout {
        anchors.fill: parent
        anchors.bottomMargin: 10
        spacing: 0//10 - topMargin already set for profilePictureRect
        
        // Rating dialog
        Dialog {
            id: rateDialog
            anchors.centerIn: parent
            width: 700; height: 500
            visible: false
            modal: true
            closePolicy: Popup.NoAutoClose | Popup.CloseOnEscape
            focus: true // The Popup will receive focus when it is shown
            
            onAccepted: {
                console.log("Ok clicked")
                if(commentTextArea.text.length > 0 && rateButtonGroup.checkedButton != null) {
                    User.rateSeller(userModel.monero_address, rateButtonGroup.checkedButton.buttonValue, commentTextArea.text)
                    commentTextArea.text = "" // clear message after sending it
                }
            }
            onRejected: {
                console.log("Cancel clicked")
                commentTextArea.text = ""
            }
            // header
            header: Rectangle {
                id: titleBar1
                color: "#323232"
                height: 40
                width: parent.width
                anchors.left: parent.left
                anchors.right: parent.right
                radius: 6
                // Rounded top corners
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.height / 2
                    color: parent.color
                }
            
                Label {
                    text: "Rate seller"
                    color: "#ffffff"
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            
                Button {
                    id: closeButton1
                    width: 25//20
                    height: this.width

                    anchors.verticalCenter: titleBar1.verticalCenter
                    anchors.right: titleBar1.right
                    anchors.rightMargin: 10
                    text: qsTr(FontAwesome.xmark)
                    contentItem: Text {  
                        text: closeButton1.text
                        color: "#ffffff"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#ff4d4d"
                        radius: 100
                    }
                    onClicked: {
                        rateDialog.reject() // manually trigger onRejected signal and close dialog too
                    }
                }
            }
            // footer
            footer: Rectangle { // footer cant have margins cuz it is an integral part of the dialog
                width: parent.width; height: 50
                color: "transparent"
                //border.color: "red" // <- for debug purposes
                RowLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: rateDialogContent.width - spacing
                    Button {
                        Layout.fillWidth: true
                        text: "Cancel"
                        onClicked: {
                            rateDialog.reject()
                        }
                    }
                
                    Button {
                        Layout.fillWidth: true
                        text: "Rate"
                        onClicked: {
                            rateDialog.accept()
                        }
                    }
                }
            }
            // background
            background: Rectangle {
                color: "white" // Change this based on theme later
                radius: 10
            }
            
            // Dialog content
            Rectangle {
                id: rateDialogContent
                anchors.centerIn: parent
                anchors.margins: 20
                width: parent.width; height: parent.height
                color: "transparent"
                //border.color: "red" // <- for debug purposes
                
                ColumnLayout {
                    anchors.fill: parent
                    Row {
                        id: rateButtonsRow
                        Layout.fillWidth: true
                        spacing: 5
                        ButtonGroup { 
                            id: rateButtonGroup
                            exclusive: true
                            onClicked: { 
                                //button.checked = true
                            }
                        }
                        Button {
                            width: commentTextArea.width / 2
                            height: 50
                            checkable: true
                            ButtonGroup.group: rateButtonGroup
                            property int buttonValue: 0
                            text: qsTr("0")
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.checked ? "red" : "#808080"
                                radius: 5
                            }
                        }
                        Button {
                            width: (commentTextArea.width / 2) - parent.spacing
                            height: 50
                            checkable: true
                            ButtonGroup.group: rateButtonGroup
                            property int buttonValue: 1
                            text: qsTr("1")
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.checked ? "green" : "#808080"
                                radius: 5
                            }
                        }
                    }
                    
                    TextArea {
                        id: commentTextArea
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: Text.Wrap
                        readOnly: (rateButtonGroup.checkedButton === null)
                        selectByMouse: true
                        focus: true // will receive focus when the Popup is shown (rateDialog must have focus set to true as well for this to work)
                        property int maximumLength: 1024
                    
                        background: Rectangle {
                            color: "lightblue"
                            radius: 5
                        }
                    }
                    
                    Label {
                        text: "Characters: " + commentTextArea.text.length
                        color: (commentTextArea.text.length > commentTextArea.maximumLength) ? "red" : "black"
                    }
                }
            }
        }
        
        // Message dialog
        Dialog {
            id: messageDialog
            anchors.centerIn: parent
            width: 700; height: 500
            visible: false
            modal: true
            closePolicy: Popup.NoAutoClose | Popup.CloseOnEscape
            focus: true // The Popup will receive focus when it is shown
            
            onAccepted: {
                console.log("Ok clicked")
                if(messageTextArea.text.length > 0) {
                    User.sendMessage(userModel.monero_address, messageTextArea.text, userModel.public_key)
                    messageTextArea.text = "" // clear message after sending it
                }
            }
            onRejected: {
                console.log("Cancel clicked")
                messageTextArea.text = ""
            }
            // header
            header: Rectangle {
                id: titleBar
                color: "#323232"
                height: 40
                width: parent.width
                anchors.left: parent.left
                anchors.right: parent.right
                radius: 6
                // Rounded top corners
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.height / 2
                    color: parent.color
                }
            
                Label {
                    text: "Message seller"
                    color: "#ffffff"
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            
                Button {
                    id: closeButton
                    width: 25//20
                    height: this.width

                    anchors.verticalCenter: titleBar.verticalCenter
                    anchors.right: titleBar.right
                    anchors.rightMargin: 10
                    text: qsTr(FontAwesome.xmark)
                    contentItem: Text {  
                        text: closeButton.text
                        color: "#ffffff"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#ff4d4d"
                        radius: 100
                    }
                    onClicked: {
                        messageDialog.reject() // manually trigger onRejected signal and close dialog too
                    }
                }
            }
            // footer
            footer: Rectangle { // footer cant have margins cuz it is an integral part of the dialog
                width: parent.width; height: 50
                color: "transparent"
                //border.color: "red" // <- for debug purposes
                RowLayout {
                    id: standardButtonsRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: messageDialogContent.width - spacing
                    Button {
                        Layout.fillWidth: true
                        text: "Cancel"
                        onClicked: {
                            messageDialog.reject()
                        }
                    }
                
                    Button {
                        Layout.fillWidth: true
                        text: "Send"
                        onClicked: {
                            messageDialog.accept()
                        }
                    }
                }
            }
            // background
            background: Rectangle {
                color: "white" // Change this based on theme later
                radius: 10
            }
            
            // Dialog content
            Rectangle {
                id: messageDialogContent
                anchors.centerIn: parent
                anchors.margins: 20
                width: parent.width; height: parent.height
                color: "transparent"
                //border.color: "red" // <- for debug purposes
                
                ColumnLayout {
                    anchors.fill: parent
                    TextArea {
                        id: messageTextArea
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: Text.Wrap
                        selectByMouse: true
                        focus: true // will receive focus when the Popup is shown (messageDialog must have focus set to true as well for this to work)
                        property int maximumLength: 470
                    
                        background: Rectangle {
                            color: "lightblue"
                            radius: 5
                        }
                    }
                    
                    Label {
                        id: characterCountLabel
                        text: "Characters: " + messageTextArea.text.length
                        color: (messageTextArea.text.length > messageTextArea.maximumLength) ? "red" : "black"
                    }
                }
            }
        }
        // Back button
        Button {
            id: backButton
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.leftMargin: 24; Layout.topMargin: 20
            implicitWidth: contentItem.contentWidth + 40; implicitHeight: contentItem.contentHeight + 20
            text: qsTr("←  Back")//"⇦  Back")
            hoverEnabled: true
            contentItem: Text {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: backButton.text
                color: "#ffffff"
            }
            background: Rectangle {
                radius: 5
                color: backButton.hovered ? NeroshopComponents.Style.neroshopPurpleColor : "#50446f"
            }
            onClicked: {
                if(productModel !== null) {
                    pageLoader.setSource("qrc:/qml/pages/ProductPage.qml", {"model": productModel})
                }
                if(messagesModel !== null) {
                    pageLoader.setSource("qrc:/qml/pages/subpages/MessagesPage.qml")
                    navBar.checkButtonByIndex(2)
                }
            }
            MouseArea {
                anchors.fill: parent
                onPressed: mouse.accepted = false
                cursorShape: Qt.PointingHandCursor
            }
        }        
        
            ColumnLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.leftMargin: 24; Layout.rightMargin: Layout.leftMargin
                Layout.topMargin: 20 
                Layout.fillWidth: true
                
                Rectangle {
                    id: profileCard
                    Layout.fillWidth: true//width: parent.width//width: 400//
                    Layout.preferredHeight: 300//height: 500//
                    color: (bannerImage.status != Image.Ready) ? "royalblue" : "transparent"
                    radius: 7

                    Image {
                        id: bannerImage
                        //source: "file:///" + "/path/to/cover_art.jpg"
                        width: parent.width; height: (parent.height - infoRect.height)//parent.height////anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        mipmap: true
                        // Apply rounded rectangle mask (radius)
                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Rectangle {
                                radius: profileCard.radius
                                width: profileCard.width
                                height: profileCard.height - infoRect.height // height is different from profileCard's height so we can't use profileCard directly or it'll cause issues
                            }
                        }
                    }
            
                    // Bottom rect
                    Rectangle {
                        id: infoRect
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left // so that margins will also apply to left and right sides
                        anchors.right: parent.right
                        width: profileCard.width//400
                        height: profileCard.height - 150//profileCard.height / 2
                        color: profilePictureRect.border.color//"#0e0e11"//"transparent"
                        radius: profileCard.radius
                    }
            
                    // Hide radius in between profileCard and infoRect
                    Rectangle {
                        width: infoRect.width - (parent.border.width * 2); height: infoRect.height / 2
                        anchors.left: parent.left; anchors.leftMargin: 0
                        anchors.right: parent.right; anchors.rightMargin: 0
                        anchors.top: infoRect.top; anchors.topMargin: -10
                        color: infoRect.color
                        border.width: parent.border.width; border.color: infoRect.color
                        radius: 0
                    }
            
                    Rectangle {
                        id: profilePictureRect
                        anchors.left: parent.left; anchors.leftMargin: 10//30
                        anchors.top: parent.top; anchors.topMargin: 50
                    
                        width: 128//Layout.preferredWidth: 128
                        height: width//Layout.preferredHeight: Layout.preferredWidth
                        color: infoRect.color// originally no color was set for this so the default was white
                        radius: 5
                        border.width: 7
                        border.color: NeroshopComponents.Style.getColorsFromTheme()[1]//"#343434"//"#808080"//"#0e0e11"//"#000000"//"#ffffff"

                        Image {
                            id: profilePicture
                            source: !userModel.hasOwnProperty("avatar") ? "https://api.dicebear.com/6.x/identicon/png?seed=%1".arg(userModel.monero_address) : "image://avatar?id=%1&image_id=%2".arg(userModel.key).arg(userModel.avatar.name)
                            anchors.centerIn: parent
                            width: parent.width - (profilePictureRect.border.width * 2); height: width
                            fillMode: Image.PreserveAspectFit
                            mipmap: true
                            asynchronous: true
                            onStatusChanged: {
                                if (profilePicture.status === Image.Error) {
                                    profilePicture.source = "https://api.dicebear.com/6.x/identicon/png?seed=%1".arg(userModel.monero_address)
                                }
                            }
                            // Apply rounded rectangle mask (radius)
                            layer.enabled: true
                            layer.effect: OpacityMask {
                                maskSource: profilePictureRect
                            }
                        }
                    }
                    // user identity column
                    Column {
                        id: nameIdColumn
                        anchors.top: profilePictureRect.bottom
                        anchors.left: profilePictureRect.left
                        anchors.leftMargin: profilePictureRect.border.width
                        // display name
                        Text {
                            text: userModel.hasOwnProperty("display_name") ? userModel.display_name : ""
                            font.pixelSize: 16//32
                            font.bold: true
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        }
                        // user id
                        TextArea {
                            text: userModel.monero_address
                            font.pixelSize: 16
                            //font.bold: true
                            color: (NeroshopComponents.Style.darkTheme) ? "#d0d0d0" : "#464646"
                            readOnly: true
                            wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width
                            selectByMouse: true
                            //background: Rectangle { color: "transparent" }
                            padding: 0; leftPadding: 0
                            width: Math.min(infoRect.width, mainWindow.minimumWidth)//200
                        }
                    }
                    // buttonsRow
                    Row {
                        id: buttonsRow
                        layoutDirection: Qt.RightToLeft
                        anchors.right: parent.right; anchors.rightMargin: 24
                        anchors.top: profilePictureRect.top
                        anchors.topMargin: profilePicture.height / 3
                        spacing: 10
                        property real buttonRadius: 6
                        property string buttonColor: infoRect.color//NeroshopComponents.Style.neroshopPurpleColor
                
                        Button {
                            width: contentItem.contentWidth + 30; height: contentItem.contentHeight + 20
                            text: qsTr("Message")
                            background: Rectangle {
                                color: buttonsRow.buttonColor
                                radius: buttonsRow.buttonRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                messageDialog.open()
                            }
                            MouseArea {
                                anchors.fill: parent
                                onPressed: mouse.accepted = false
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                        
                        Button {
                            width: contentItem.contentWidth + 30; height: contentItem.contentHeight + 20
                            text: qsTr("Rate")
                            background: Rectangle {
                                color: buttonsRow.buttonColor
                                radius: buttonsRow.buttonRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                rateDialog.open()
                            }
                            MouseArea {
                                anchors.fill: parent
                                onPressed: mouse.accepted = false
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                    // stats column
                    Column {
                        id: statsRow
                        anchors.top: nameIdColumn.bottom
                        anchors.topMargin: 10
                        anchors.left: profilePictureRect.left
                        anchors.leftMargin: profilePictureRect.border.width
                        property int textIconSpacing: 5
                        property real iconSize: 24
                        width: profileCard.width
                        // stats row
                        Row {
                            id: statsRowActual
                            spacing: 100
                            // reputation
                            Column {
                                spacing: statsRow.textIconSpacing
                                // Deprecated/Replaced with hint (tooltip). Remove this soon!
                                /*Text {
                                    text: "Reputation"
                                    font.pixelSize: 16//32
                                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                }*/
                                
                                Row { // place thumbs in this row
                                    spacing: 5
                                Rectangle {
                                    //anchors.verticalCenter: parent.verticalCenter
                                    //anchors.left: parent.left; anchors.leftMargin: width / 2
                                    id: reputationRect
                                    width: 100; height: 32
                                    color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#efefef"
                                    border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                    radius: 3
                                    property bool hovered: false
                                    
                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 5
                                        Item {
                                            id: ratingIconRect
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: childrenRect.width
                                            height: childrenRect.height
                                    
                                            Image {
                                                id: ratingIcon
                                                source: "qrc:/assets/images/rating.png"
                                                width: statsRow.iconSize; height: width
                                                mipmap: true
                                            }
                                
                                            ColorOverlay {
                                                anchors.fill: ratingIcon
                                                source: ratingIcon
                                                color: "#ffd700"//"#e6c200"
                                                visible: ratingIcon.visible
                                            }
                                        }
                                    
                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: Backend.getSellerReputation(ratingsModel) + "%"
                                            font.pixelSize: 16
                                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                        }
                                    }
                                    
                                    NeroshopComponents.Hint {
                                        visible: parent.hovered
                                        height: contentHeight + 20; width: contentWidth + 20
                                        text: qsTr("Reputation")
                                        pointer.visible: false;// delay: 0
                                    }
                                    
                                    MouseArea { 
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onEntered: parent.hovered = true
                                        onExited: parent.hovered = false
                                    }
                                }
                                // thumbs up/thumbs down
                        Rectangle {
                            anchors.top: parent.children[0].top
                            width: 50 + thumbsUpCountText.contentWidth; height: reputationRect.height
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#efefef"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            radius: 3
                            Row {
                                anchors.centerIn: parent
                                spacing: 5
                                Item {
                                    id: thumbsUpImageItem
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: childrenRect.width//statsRow.iconSize
                                            height: childrenRect.height
                                            visible: (thumbsUpImage.status === Image.Ready)
                                    Image {
                                        id: thumbsUpImage
                                        source: "qrc:/assets/images/thumbs_up.png"
                                        width: 18; height: width
                                        mipmap: true
                                    }
                            
                                    ColorOverlay {
                                        id: thumbsUpImageOverlayer
                                        anchors.fill: thumbsUpImage
                                        source: thumbsUpImage
                                        color: "green"//"#506a1a"
                                        visible: thumbsUpImage.visible
                                    }
                                }
                                Text {
                                    id: fallbackThumbsUpIcon
                                    anchors.verticalCenter: parent.verticalCenter
                                    text:qsTr(FontAwesome.thumbsUp)
                                    visible: !thumbsUpImageItem.visible
                                    font.bold: true
                                    color: thumbsUpImageOverlayer.color
                                }
                                Text {
                                    id: thumbsUpCountText
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: Backend.getSellerGoodRatings(ratingsModel)
                                            font.pixelSize: 16
                                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                        }
                            }
                          } // thumbsUp Rect
                          // thumbs down rect
                          Rectangle {
                            anchors.top: parent.children[0].top
                            width: 50 + thumbsDownCountText.contentWidth; height: reputationRect.height
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#efefef"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            radius: 3
                            Row {
                                anchors.centerIn: parent
                                spacing: 5
                                Item {
                                    id: thumbsDownImageItem
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: childrenRect.width//statsRow.iconSize
                                            height: childrenRect.height
                                            visible: (thumbsDownImage.status === Image.Ready)
                                    Image {
                                        id: thumbsDownImage
                                        source: "qrc:/assets/images/thumbs_down.png"
                                        width: 18 + 2; height: width
                                        mipmap: true
                                    }
                            
                                    ColorOverlay {
                                        id: thumbsDownImageOverlayer
                                        anchors.fill: thumbsDownImage
                                        source: thumbsDownImage
                                        color: "red"//"firebrick"
                                        visible: thumbsDownImage.visible
                                    }
                                }
                                Text {
                                    id: fallbackThumbsDownIcon
                                    anchors.verticalCenter: parent.verticalCenter
                                    text:qsTr(FontAwesome.thumbsDown)
                                    visible: !thumbsDownImageItem.visible
                                    font.bold: true
                                    color: thumbsDownImageOverlayer.color
                                }
                                Text {
                                    id: thumbsDownCountText
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: Backend.getSellerBadRatings(ratingsModel)
                                            font.pixelSize: 16
                                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                        }
                            }
                          } // thumbsDown Rect
                                
                                } // row containing both thumbs and reputation
                            } // column for reputation text/reputation stats (can be safely removed)
                            
                        } // end of statsRow
                    } // end of statsCol
            
                    // TODO: show mail letter icon for email, location icon for location, Link icon for website
                } // profileCard (Rectangle)
            } // ColumnLayout
        
            // Tabs
            Rectangle {
                id: tabsRect
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.topMargin: 20; 
                Layout.leftMargin: 24; Layout.rightMargin: Layout.leftMargin
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                color: "royalblue"//NeroshopComponents.Style.neroshopPurpleColor//infoRect.color
                radius: 0//profileCard.radius
                Row {       
                    id: tabButtonRow
                    anchors.bottom: parent.bottom//anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left; anchors.leftMargin: 10
                    spacing: 0////15
                    property real buttonRadius: 5
                    property string buttonCheckedColor: tabLayout.tabItemColor
                    property string buttonUncheckedColor: tabsRect.color
                    Button {
                        id: listingsTabButton
                        width: (tabsRect.width / tabButtonRow.children.length) - tabButtonRow.anchors.leftMargin/*!listingsCountRect.visible ? 100 : 100 + listingsCountText.contentWidth*/; height: 40
                        text: qsTr("Listings")
                        autoExclusive: true
                        checkable: true
                        checked: true // default
                        background: Rectangle {
                            color: parent.checked ? tabButtonRow.buttonCheckedColor : tabButtonRow.buttonUncheckedColor
                            radius: tabButtonRow.buttonRadius
                            
                            // To hide bottom radius
                            Rectangle {
                                anchors.left: parent.left
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 5
                                color: parent.color//"pink"// <- for testing
                            }
                        }
                        contentItem: Item {
                            anchors.fill: parent
                            Row { 
                                anchors.centerIn: parent
                                spacing: 10
                                Text {
                                    text: listingsTabButton.text
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                    font.bold: true
                                }
                            
                                Rectangle {
                                    id: listingsCountRect
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: listingsCountText.contentWidth + 15; height: 20
                                    color: "#101010"//tabLayout.tabItemColor
                                    radius: 3
                                    visible: Number(listingsCountText.text) > 0 && !listingsTabButton.checked
                                    Text {
                                        id: listingsCountText
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: (profilePage.listingsModel != null) ? profilePage.listingsModel.length : "0"
                                        color: "#ffffff"
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                        onClicked: {
                            tabLayout.currentIndex = 0
                        }
                    }
                    
                    Button {
                        id: ratingsTabButton
                        width: (tabsRect.width / tabButtonRow.children.length) - tabButtonRow.anchors.leftMargin/*!ratingsCountRect.visible ? 100 : 100 + ratingsCountText.contentWidth*/; height: 40
                        text: qsTr("Ratings")
                        autoExclusive: true
                        checkable: true
                        background: Rectangle {
                            color: parent.checked ? tabButtonRow.buttonCheckedColor : tabButtonRow.buttonUncheckedColor
                            radius: tabButtonRow.buttonRadius
                            
                            // To hide bottom radius
                            Rectangle {
                                anchors.left: parent.left
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 5
                                color: parent.color
                            }
                        }
                        contentItem: Item {
                            anchors.fill: parent
                            Row {
                                anchors.centerIn: parent
                                spacing: 10
                                Text {
                                    text: ratingsTabButton.text
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                    font.bold: true
                                }
                            
                                Rectangle {
                                    id: ratingsCountRect
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: ratingsCountText.contentWidth + 15; height: 20
                                    color: "#101010"//tabLayout.tabItemColor
                                    radius: 3
                                    visible: Number(ratingsCountText.text) > 0 && !ratingsTabButton.checked
                                    Text {
                                        id: ratingsCountText
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: Backend.getSellerRatingsCount(ratingsModel)
                                        color: "#ffffff"
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                        onClicked: {
                            tabLayout.currentIndex = 1
                        }
                    }
                }
            }
        // StackLayout
        StackLayout {
            id: tabLayout
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.leftMargin: 24; Layout.rightMargin: Layout.leftMargin
            Layout.preferredWidth: tabsRect.width; Layout.fillHeight: true
            currentIndex: 0
            property string tabItemColor: NeroshopComponents.Style.getColorsFromTheme()[1]//"#101010"//NeroshopComponents.Style.getColorsFromTheme()[0]//"transparent"//""
            Rectangle {
                id: listingsTabItem
                color: tabLayout.tabItemColor
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                clip: true
                
                GridView {
                    id: listingsGrid
                    anchors.fill: parent
                    property real margins: 15
                    anchors.topMargin: listingsGrid.margins; anchors.bottomMargin: anchors.topMargin
                    anchors.leftMargin: listingsGrid.margins; anchors.rightMargin: anchors.leftMargin
                    cellWidth: 200; cellHeight: 128//200
                    property int spacing: 5
                    model: profilePage.listingsModel
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    delegate: Rectangle {
                        id: productBox
                        width: listingsGrid.cellWidth - listingsGrid.spacing
                        height: listingsGrid.cellHeight - listingsGrid.spacing
                        color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#181a1b") : "#c9c9cd"//"transparent"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"
                        border.width: 0
                        radius: 5
                        
                        Rectangle {
                            id: productImageRect
                            anchors.fill: parent
                            anchors.margins: parent.border.width
                            color: "#ffffff"
                            radius: parent.radius
                             
                            Image {
                                id: productImage
                                source: "image://listing?id=%1&image_id=%2".arg(modelData.key).arg(modelData.product_images[0].name)//"file:///" + modelData.product_image_file//"qrc:/assets/images/image_gallery.png"
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectFit
                                mipmap: true
                                asynchronous: true                    
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton
                                onEntered: {
                                    productBox.border.width = 1
                                }
                                onExited: {
                                    productBox.border.width = 0
                                }
                                onClicked: { 
                                    navBar.uncheckAllButtons() // Uncheck all navigational buttons
                                    pageLoader.setSource("qrc:/qml/pages/ProductPage.qml", { "model": modelData })
                                }
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: ratingsTabItem
                color: tabLayout.tabItemColor
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                
                Flickable {
                    anchors.fill: parent
                    contentWidth: parent.width; contentHeight: ratingsList.contentHeight + (ratingsList.margins * 2) // to fill the top + bottom padding
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    ListView {
                        id: ratingsList
                        /*property int margins: 10
                        anchors.left: parent.left; anchors.top: parent.top
                        anchors.margins: margins*/
                        anchors.fill: parent//width: parent.width
                        property real margins: 15
                        anchors.topMargin: ratingsList.margins; anchors.bottomMargin: anchors.topMargin
                        model: profilePage.ratingsModel
                        spacing: 10
                        property real delegateHeight: 100
                        delegate: Rectangle {
                            anchors.left: parent.left; anchors.right: parent.right
                            anchors.leftMargin: ratingsList.margins; anchors.rightMargin: anchors.leftMargin
                            width: parent.width; height: Math.max(ratingsList.delegateHeight, ratingsList.delegateHeight + commentsLabel.height)
                            color: "transparent"//(NeroshopComponents.Style.darkTheme) ? "#101010" : "#efefef"//index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
                            radius: 6
                            property string textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            // Display review data using Text elements
                            Component.onCompleted: console.log("commentsLabel.height",commentsLabel.height)
                            Rectangle {
                                id: avatarRect
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.height - 20; height: width
                                Image {
                                    anchors.centerIn: parent
                                    width: parent.width - (avatarRect.border.width * 2); height: width
                                    fillMode: Image.PreserveAspectCrop
                                    mipmap: true
                                    source: "https://api.dicebear.com/6.x/identicon/png?seed=%1".arg(modelData.rater_id)
                                }
                            }
                            Column {
                                anchors.left: avatarRect.right
                                anchors.leftMargin: 10
                                anchors.top: avatarRect.top
                                width: parent.width - (avatarRect.anchors.leftMargin + avatarRect.width + anchors.leftMargin + parent.anchors.rightMargin)
                                //height: parent.height
                                spacing: 30//15
                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 0
                                    text: modelData.rater_id
                                    color: parent.parent.textColor
                                    font.bold: true
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                    width: parent.width
                                }
                            
                                Text {//TextArea {
                                    id: commentsLabel
                                    //anchors.left: parent.left
                                    //anchors.leftMargin: 0
                                    text: modelData.comments// reviews cannot surpass 1024 bytes
                                    color: parent.parent.textColor
                                    ////readOnly: true
                                    wrapMode: Text.WordWrap////Text.Wrap//
                                    width: parent.width
                                    height: (text.length >= 1024) ? 200 : 100//Math.min(100, (text.length * 10) / parent.height)//200
                                    ////background: Rectangle { color: "transparent" }
                                    elide: Text.ElideRight
                                }
                            } // Column
                            // TODO: add timestamp too                    
                            Text {
                                id: scoreLabel
                                anchors.right: parent.right
                                anchors.rightMargin: 10
                                //anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 10
                                text: "Score: " + modelData.score
                                color: parent.textColor
                            }
                        } // delegate
                    } // ListView
                } // Flickable
            } // StackLayout Item
        } // StackLayout
    } // root ColumnLayout
} // end of Page
