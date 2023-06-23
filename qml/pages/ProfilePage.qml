// This page represents the user's profile where all the user's listings, contact information, etc. will be displayed
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12 // ColorOverlay

import "../components" as NeroshopComponents

Page {
    id: profilePage
    background: Rectangle {
        color: "transparent"
    }

    ColumnLayout {
        width: parent.width
        spacing: 0//10 - topMargin already set for profilePictureRect
                
        RowLayout {
            spacing: 24
        
            Column {
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.leftMargin: 24; Layout.topMargin: 20
                
                Rectangle {
                    id: profileCard
                    width: 400//Layout.fillWidth: true//width: parent.width
                    height: 500//Layout.preferredHeight: 200
                    color: (bannerImage.status != Image.Ready) ? "royalblue" : "transparent"
                    radius: 7

                    Image {
                        id: bannerImage
                        source: "file:///" + "/home/sid/Downloads/monero-support-your-local-cypherpunk-1920x1080.png"//"path/to/cover_art.jpg"
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
                        radius: 5
                        border.width: 7
                        border.color: NeroshopComponents.Style.getColorsFromTheme()[1]//"#343434"//"#808080"//"#0e0e11"//"#000000"//"#ffffff"

                        Image {
                            id: profilePicture
                            source: "file:///" + "/home/sid/Downloads/monero-geometric-logo-800x800.png"//"path/to/profile_picture.jpg"
                            anchors.centerIn: parent
                            width: parent.width - profilePictureRect.border.width; height: width
                            fillMode: Image.PreserveAspectFit
                            mipmap: true
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
                        anchors.leftMargin: 3
                        //anchors.horizontalCenter: profilePictureRect.horizontalCenter
                        //anchors.top: parent.top; anchors.topMargin: 10//20//anchors.verticalCenter: parent.verticalCenter
                        // display name
                        Text {
                            text: "layter" // Replace with actual user name
                            font.pixelSize: 16//32
                            //font.bold: true
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        }
                        // user id
                        TextArea {
                            text: "5AWjbNUBf2EbbCw2v6ChrJUCdeRjfpcH5Y63wpWz37X6ZEiU9gvGeFqQpZczeVtZnd479FE4SDvKy7yF8ozj99QTRzcTY3a" // Replace with actual user ID
                            font.pixelSize: 16
                            //font.bold: true
                            color: "dimgray"
                            readOnly: true
                            wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width
                            selectByMouse: true
                            //background: Rectangle { color: "transparent" }
                            padding: 0; leftPadding: 0
                            width: 200
                        }
                    }
                    // stats column
                    Column {
                        id: statsRow
                        anchors.top: nameIdColumn.bottom
                        anchors.topMargin: 10
                        anchors.left: profilePictureRect.left
                        anchors.leftMargin: 1
                        property int textIconSpacing: 5
                        property real iconSize: 24
                        // stats row
                        Row {
                            spacing: 100
                            // reputation
                            Column {
                                spacing: statsRow.textIconSpacing
                                Text {
                                    text: "Reputation"
                                    font.pixelSize: 16//32
                                    //font.bold: true
                                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                }
                                
                                Rectangle {
                                    //anchors.verticalCenter: parent.verticalCenter
                                    //anchors.left: parent.left; anchors.leftMargin: width / 2
                                    width: 100; height: 26
                                    color: "transparent"
                                    border.color: "#ffffff"
                                    radius: 3
                                    
                                    Row {
                                        anchors.fill: parent
                                        spacing: 5
                                        Item {
                                            Image {
                                                id: ratingIcon
                                                source: "qrc:/images/rating.png"
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
                                            text: "97%"
                                            font.pixelSize: 16
                                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                        }
                                    }
                                }
                            }
                        // products
                        Column {
                            spacing: statsRow.textIconSpacing
                            Text {
                                text: "Products" // Replace with actual user name
                                font.pixelSize: 16//32
                                //font.bold: true
                                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            }
                            
                            Rectangle {
                                    //anchors.verticalCenter: parent.verticalCenter
                                    //anchors.left: parent.left; anchors.leftMargin: width / 2
                                    width: 64; height: 26
                                    color: "transparent"
                                    border.color: "#ffffff"
                                    radius: 3
                                    Image {
                                        id: productIcon
                                        source: "qrc:/images/open_parcel.png"
                                        width: statsRow.iconSize; height: width
                                        mipmap: true
                                    }
                            
                                    ColorOverlay {
                                        anchors.fill: productIcon
                                        source: productIcon
                                        color: "#4169e1"
                                        visible: productIcon.visible
                                    }
                            }
                        }
                    }
                }
                    /*Column {
                        Row {
                                    ////anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left; anchors.leftMargin: width / 2
                                    //width: 32; height: 32
                                    //color: "#fffbe5"
                                    //radius: 3//50
                                    Image {
                                        id: ratingIcon
                                        source: "qrc:/images/rating.png"
                                        width: 32; height: 32
                                        anchors.centerIn: parent
                                    }
                            
                                    ColorOverlay {
                                        anchors.fill: ratingIcon
                                        source: ratingIcon
                                        color: "#ffd700"//"#e6c200"
                                        visible: ratingIcon.visible
                                    }
                        }
                    }*/
                ////}
            
            /*Row {
                layoutDirection: Qt.RightToLeft
                anchors.right: parent.right; anchors.rightMargin: 30
                anchors.verticalCenter: parent.verticalCenter // TODO: make this bottom/bottomPadding
                
                Button {
                    width: contentItem.contentWidth + 20; height: contentItem.contentHeight + 20
                    text: qsTr("Message")
                    background: Rectangle {
                        color: NeroshopComponents.Style.neroshopPurpleColor
                        radius: 3
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                    }
                    onClicked: {}
                }
            }*/ // Row 2
            // TODO: show stats and reputation like good ratings(thumbs up), bad ratings (thumbs down)
            // Mail letter icon for email, location icon for location, Link icon for website
            } // Rectangle
        } // Column
        
            // Tabs
            Rectangle {
                id: tabsRect
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                Layout.topMargin: 20; Layout.rightMargin: 24;
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                color: "#343434"
                radius: profileCard.radius
                Row {       
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left; anchors.leftMargin: 10
                    spacing: 15
                    Button {
                        width: contentItem.contentWidth + 20; height: contentItem.contentHeight + 20
                        text: qsTr("Listing")
                        background: Rectangle {
                            color: NeroshopComponents.Style.neroshopPurpleColor
                            radius: 3
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                        }
                        onClicked: {}
                    }
                    
                    Button {
                        width: contentItem.contentWidth + 20; height: contentItem.contentHeight + 20
                        text: qsTr("Ratings")
                        background: Rectangle {
                            color: NeroshopComponents.Style.neroshopPurpleColor
                            radius: 3
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                        }
                        onClicked: {}
                    }
                }
            }
        } // RowLayout
        
        /*NeroshopComponents.TabBar {
            id: tabBar
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            model: ["Store", "Ratings", "Details"]
            color0: NeroshopComponents.Style.neroshopPurpleColor
            Component.onCompleted: {
                buttonAt(0).checked = true
            }
        }*/
        /*TabBar {
            id: tabBar
            width: parent.width
            TabButton {
                text: qsTr("Store") // Listings
                width: text.length * 10
            }
            TabButton {
                text: qsTr("Ratings")
            }
            TabButton {
                text: qsTr("Details") // About
            }
        }*/

        /*StackView {

        }*/
    }    
}
