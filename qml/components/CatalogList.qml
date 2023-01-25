import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12

import FontAwesome 1.0
import "." as NeroshopComponents

Column {
    id: catalogList
    width: childrenRect.width/*boxWidth*/; height: (childrenRect.height * catalogListRepeater.count)//(boxHeight * catalogListRepeater.count) // The height is the height of all the models in the listview combined
    spacing: 5
    property bool hideProductDetails: false // hides product name, price, and star ratings if set to true
    property real boxWidth: 600//parent.width
    property real boxHeight: 300
    property alias count: catalogListRepeater.count
    //Component.onCompleted: {console.log("catalogList children height",childrenRect.height)}
             
    Repeater {
        id: catalogListRepeater
        model: Backend.getListings()//10
        delegate: Rectangle {
            id: productBox
            width: catalogList.boxWidth; height: catalogList.boxHeight // The height of each individual model item/ list element
            color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e" : "#a0a0a0"
            border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            border.width: 0
            radius: 5
            //clip: true

            Rectangle {
                id: productImageRect
                anchors.top: parent.top
                anchors.left: parent.left // so that margins will also apply to left and right sides
                anchors.margins: parent.border.width
                anchors.verticalCenter: parent.verticalCenter
                width: (parent.width / 3) + 20; height: parent.height
                color: "#ffffff"//"transparent"
                radius: parent.radius
                             
                Image {
                    id: productImage
                    source: "file:///" + modelData.product_image_file//"qrc:/images/image_gallery.png"
                    anchors.centerIn: parent
                    width: 128; height: 128
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
                            ////pageLoader.source = "../pages/ProductPage.qml"
                        }
                    }                    
                }
            } // ProductImageRect
        }     
    }
}
