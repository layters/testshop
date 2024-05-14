// this page will display all products that have been searched for by the user in the search field
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import FontAwesome 1.0
import neroshop.CurrencyExchangeRates 1.0

import "." as NeroshopComponents

GridView {
    id: catalogGrid
    // Bug detected: binding the catalogGrid's height to a variable will not work
    width: 910; height: 910//width: (cellWidth * columns) + (spacing * (columns - 1)); height: (cellHeight * rows) + (spacing * (rows - 1))
    cellWidth: 300//250
    cellHeight: (settingsDialog.hideProductDetails) ? 300 : 400
    property int spacing: 5//rowSpacing: 5; columnSpacing: 5
    property int columns: Math.round(width / cellWidth)
    property int rows: Math.round(height / cellHeight)
    Component.onCompleted: {
        width = contentItem.childrenRect.width
        height = contentItem.childrenRect.height
        console.log("grid size", width, height)
        console.log("grid columns", columns)
        console.log("grid rows", rows)
    }
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
    }    
    function getBox(index) { // or get_item(index)?
        return catalogGridRepeater.itemAt(index);
    }
    function getBoxCount() {
        return catalogGridRepeater.count; // count is really just the number of items in the model :O
    }
    model: Backend.getListings()//(rows * columns)//// rows and columns already set so this is useless (I think)
    // product box (GridBox)
    delegate: Rectangle { // delegates have a readonly "index" property that indicates the index of the delegate within the repeater
        id: productBox
        visible: true
        width: catalogGrid.cellWidth-catalogGrid.spacing
        height: catalogGrid.cellHeight-catalogGrid.spacing
        color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#181a1b") : "#c9c9cd"//"#e6e6e6"//"#f0f0f0"
        border.color: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
        border.width: 0
        radius: 5
        clip: true // So that productNameText will be clipped to the Rectangle's bounding rectangle and will not go past it
        //anchors.right: this.right; anchors.rightMargin: 5
        // Hide radius at bottom border
        Rectangle {
            width: productImageRect.width - (parent.border.width * 2); height: productImageRect.height / 2
            anchors.left: parent.left; anchors.leftMargin: parent.border.width
            anchors.right: parent.right; anchors.rightMargin: parent.border.width
            anchors.bottom: productImageRect.bottom; anchors.bottomMargin: -2
            color: productImageRect.color//"blue"
            border.width: parent.border.width; border.color: productImageRect.color
            radius: 0
        }
                                                
        Rectangle {
            id: productImageRect
            anchors.top: parent.top
            anchors.left: parent.left // so that margins will also apply to left and right sides
            anchors.right: parent.right
            anchors.margins: parent.border.width
            width: parent.width; height: (settingsDialog.hideProductDetails) ? 197.5 : (parent.height / 2)// + 30//=130
            color: "#ffffff"//"transparent"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            radius: parent.radius
                             
            Image {
                id: productImage
                source: "image://listing?id=%1&image_id=%2".arg(modelData.key).arg(modelData.product_images[0].name)//"file:///" + modelData.product_image_file//"qrc:/assets/images/image_gallery.png"
                anchors.centerIn: parent
                width: 192; height: width
                fillMode: Image.PreserveAspectFit//Image.Stretch
                mipmap: true
                asynchronous: true
                onStatusChanged: {
                    if (productImage.status === Image.Error) {
                        // Handle the error by displaying a fallback or placeholder image
                        source = "image://listing?id=%1&image_id=%2".arg(modelData.key).arg("thumbnail.jpg")
                    }
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
                        pageStack.pushPageWithProperties("qrc:/qml/pages/ProductPage.qml", { "model": modelData, "starModel": starsRect.product_ratings_model })
                    }
                    cursorShape: Qt.PointingHandCursor
                }                    
            }
        }
            
        Image {
            id: verifiedPurchaseIcon
            source: "qrc:/assets/images/paid.png"//neroshopResourceDir + "/paid.png"
            visible: false // TODO: only show this icon if item has been purchased previously (since orders are encrypted, only the user can see this)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
                
            height: 24; width: 24 // has no effect since image is scaled
            fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping            
            mipmap: true

            NeroshopComponents.Hint {
                id: verifiedPurchaseIconHint
                visible: verifiedPurchaseIconMouse.hovered ? true : false
                text: qsTr("You've previously purchased this item")
                pointer.visible: false
                delay: 500; timeout: 3000 // hide after 3 seconds
            }        
                    
            HoverHandler {
                id: verifiedPurchaseIconMouse
                acceptedDevices: PointerDevice.Mouse
            }
        }
        ColorOverlay {
            anchors.fill: verifiedPurchaseIcon
            source: verifiedPurchaseIcon
            color: "#1e509b"//"#336699"// // activeColor
            visible: verifiedPurchaseIcon.visible
        }            
                                
        Button { // heartIconButton must be drawn over the productImage
            id: heartIconButton
            text: (disabled) ? qsTr("Add to favorites") : qsTr("Remove from favorites")
            display: AbstractButton.IconOnly // will only show the icon and not the text
            hoverEnabled: true
            ////containmentMask: this.icon // When the mouse is pointing at the icon's bounding box instead of the button's then the border will appear
            //width: 24; height: 24
            anchors.right: parent.right
            anchors.rightMargin: 5//10
            anchors.top: parent.top
            anchors.topMargin: 5//10
            property bool disabled: !User.hasFavorited(modelData.key)
            icon.source: "qrc:/assets/images/heart.png"
            icon.color: heartIconButton.disabled ? "#808080" : "#e05d5d"
            icon.height: 24; icon.width: 24
            background: Rectangle {
                color: "transparent"
                radius: 3//0
                border.color: parent.hovered ? (parent.disabled ? "#808080" : "#e05d5d") : "transparent"
            }
            onClicked: { 
                if(disabled) {
                    disabled = false
                    icon.color = "#e05d5d"
                    User.addToFavorites(modelData.key)
                }
                else {
                    disabled = true
                    icon.color = "#808080"
                    User.removeFromFavorites(modelData.key)
                }
            }
            MouseArea {
                anchors.fill: parent
                onPressed: mouse.accepted = false
                cursorShape: Qt.PointingHandCursor
            }
        }
        // Product details
        ColumnLayout {
            id: productDetailsColumn
            anchors.left: parent.left
            anchors.top: productImageRect.bottom
            anchors.right: parent.right
            anchors.margins: 10
            width: parent.width// Layout should automatically set the height based on its children's height
            spacing: 5
                
            TextArea {
                id: productNameText
                Layout.alignment: (!settingsDialog.gridDetailsAlignCenter) ? 0 : Qt.AlignHCenter
                width: parent.width
                property int maximumHeight: 100
                text: qsTr(modelData.product_name)//qsTr("Product name")
                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                visible: !settingsDialog.hideProductDetails
                readOnly: true
                wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width
                selectByMouse: true
                //font.bold: true
                font.pointSize: 13
                background: Rectangle { color: "transparent" }
                padding: 0; leftPadding: 0
                Component.onCompleted: {
                    if (contentHeight > productNameText.maximumHeight) {
                        console.log("contentHeight has exceeded maximumHeight", contentHeight)
                        font.pointSize = (contentHeight >= 140) ? 9 : Qt.application.font.pointSize
                    }
                    //console.log("productNameText.height",productNameText.height)
                }
                //onClicked://onFocusChanged: {
                    //if(activeFocus) pageLoader.setSource("qrc:/qml/pages/ProductPage.qml")
                //}
            }
                                
            Column {
                Layout.alignment: (!settingsDialog.gridDetailsAlignCenter) ? 0 : Qt.AlignHCenter////Layout.fillWidth: true
                spacing: 5//10
                Row {
                    spacing: 5
                                
                    Image {
                        id: moneroSymbol
                        source: "qrc:/assets/images/monero_symbol_white.png"
                        visible: priceMonero.visible
                        width: 24; height: 24
                        fillMode:Image.PreserveAspectFit
                        mipmap: true
                    }
                                
                    TextField { // Allows us to copy the string unlike Text
                        id: priceMonero
                        text: qsTr("%1").arg(CurrencyExchangeRates.convertToXmr(Number(modelData.price), modelData.currency).toFixed(Backend.getCurrencyDecimals("XMR")))//.arg("XMR") // TODO: allow users to specificy their preferred number of digits
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        visible: (settingsDialog.catalogPriceBox.currentIndex == 1) ? false : true//!settingsDialog.hideProductDetails
                        anchors.verticalCenter: moneroSymbol.verticalCenter
                        font.bold: true
                        font.pointSize: 16
                        readOnly: true
                        selectByMouse: true
                        background: Rectangle {
                            color: "transparent"
                        }
                        padding: 0; leftPadding: 0 // With the padding at zero, we can freely set the Row spacing
                    }
                }
                        
                TextField {
                    id: priceFiat
                    states: [
                        State {
                            name: "left"
                            AnchorChanges {
                                target: priceFiat
                                anchors.left: parent.left
                            }
                        },
                        State {
                            name: "centered"
                            AnchorChanges {
                                target: priceFiat
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }                        
                    ]
                    state: (!settingsDialog.gridDetailsAlignCenter) ? "left" : "centered"
                    text: qsTr("%1%2 %3").arg(Backend.getCurrencySign(modelData.currency)).arg(Number(modelData.price).toFixed(Backend.getCurrencyDecimals(modelData.currency))).arg(modelData.currency)
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: !priceMonero.visible ? true : false
                    font.pointSize: !priceMonero.visible ? 16 : Qt.application.font.pointSize
                    visible: (settingsDialog.catalogPriceBox.currentIndex == 2) ? false : true//!settingsDialog.hideProductDetails
                    readOnly: true
                    selectByMouse: true
                    background: Rectangle {
                        color: "transparent"
                    }
                    padding: 0; leftPadding: 0 // With the padding at zero, we can freely set the Row spacing
                } 
            }
            // TODO: Ratings stars and ratings count
            Rectangle {
                id: starsRect
                color: "transparent"
                //border.color: "blue" // <- for debug
                Layout.alignment: (!settingsDialog.gridDetailsAlignCenter) ? 0 : Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                property bool hovered: false
                property var product_ratings_model: Backend.getProductRatings(modelData.listing_uuid)
                property real avg_stars: Backend.getProductAverageStars(starsRect.product_ratings_model)
                property int star_ratings_count: Backend.getProductStarCount(starsRect.product_ratings_model)
                Row { 
                    id: starsRow
                    //spacing: 5
                    Repeater {
                        model: 5
                        delegate: Text {
                            text: (starsRect.star_ratings_count <= 0) ? qsTr(FontAwesome.star) : ((starsRect.avg_stars > index && starsRect.avg_stars < (index + 1)) ? qsTr(FontAwesome.starHalfStroke) : qsTr(FontAwesome.star)) // not sure?
                            color: ((index + 1) > Math.ceil(starsRect.avg_stars) || (starsRect.star_ratings_count <= 0)) ? "#777" : "#ffb344" // good!
                            font.bold: true
                            font.family: FontAwesome.fontFamily
                        }       
                    }
                    Text {
                        visible: !settingsDialog.hideProductDetails
                        anchors.verticalCenter: parent.children[0].verticalCenter
                        text: qsTr(" %1").arg((starsRect.star_ratings_count <= 0) ? "" : starsRect.star_ratings_count)
                        color: "#4169e1"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.pointSize: 10
                    }
                }
                NeroshopComponents.Hint {
                    x: starsMouseArea.mouseX; y: starsRect.height + 10//x: starsRect.width + 10; y: (starsRect.height - height) / 2
                    visible: parent.hovered
                    height: contentHeight + 30; width: contentWidth + 30
                    text: (!settingsDialog.hideProductDetails || (starsRect.star_ratings_count <= 0)) ? qsTr("%1 out of 5 stars").arg(starsRect.avg_stars) : qsTr("%1 out of 5 stars\n%2 total ratings").arg(starsRect.avg_stars).arg(starsRect.star_ratings_count)
                    pointer.visible: false
                    timeout: 5000
                }
                MouseArea { 
                    id: starsMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.hovered = true
                    onExited: parent.hovered = false
                }
            }
        } // ColumnLayout
    } // delegate
} // Catalog View (grid)
