import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12

import FontAwesome 1.0
import "." as NeroshopComponents

Column {
    id: catalogList
    width: boxWidth; height: (boxHeight * count) + (spacing * (count - 1))//(boxHeight * catalogListRepeater.count) // The height is the height of all the models in the listview combined
    spacing: 5
    property bool hideProductDetails: false // hides product name, price, and star ratings if set to true
    property real boxWidth: 910
    property real boxHeight: 300
    property alias count: catalogListRepeater.count
    property alias model: catalogListRepeater.model
    //Component.onCompleted: {console.log("catalogList children height",childrenRect.height)}
             
    Repeater {
        id: catalogListRepeater
        model: Backend.getListings()//10
        delegate: Rectangle {
            id: productBox
            width: catalogList.boxWidth; height: catalogList.boxHeight // The height of each individual model item/ list element
            color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#1d1d1d") : "#c9c9cd"//"#f0f0f0"
            border.color: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            border.width: 0
            radius: 5
            //clip: true
            // Hide radius at right border
            Rectangle {
                width: productImageRect.width / 2; height: productImageRect.height - (parent.border.width * 2)
                anchors.top: parent.top; anchors.topMargin: parent.border.width
                anchors.bottom: parent.bottom; anchors.bottomMargin: parent.border.width
                anchors.right: productImageRect.right; anchors.rightMargin: -2
                color: productImageRect.color//"blue"
                border.width: parent.border.width; border.color: productImageRect.color
                radius: 0
            }

            Rectangle {
                id: productImageRect
                anchors.top: parent.top
                anchors.left: parent.left // so that margins will also apply to left and right sides
                anchors.bottom: parent.bottom
                anchors.margins: parent.border.width
                width: (parent.width / 3); height: parent.height
                color: "#ffffff"//"transparent"
                radius: parent.radius
                             
                Image {
                    id: productImage
                    source: "file:///" + modelData.product_image_file//"qrc:/images/image_gallery.png"
                    anchors.centerIn: parent
                    width: 192; height: 192
                    fillMode: Image.PreserveAspectFit
                    mipmap: true
                    asynchronous: true
                    
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onEntered: {
                            catalogListRepeater.itemAt(index).border.width = 1
                        }
                        onExited: {
                            catalogListRepeater.itemAt(index).border.width = 0
                        }
                        onClicked: { 
                            navBar.uncheckAllButtons() // Uncheck all navigational buttons
                            console.log("Loading product page ...");
                            pageLoader.setSource("qrc:/qml/pages/ProductPage.qml")////, { "listingId": modelData.listing_uuid })
                        }
                    }                    
                }
            } // ProductImageRect
        }     
    }
}
