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
        color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#1d1d1d") : "#c9c9cd"//"#e6e6e6"//"#f0f0f0"
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
                source: "file:///" + modelData.product_image_file//"qrc:/images/image_gallery.png"
                anchors.centerIn: parent
                width: 192; height: width
                fillMode: Image.PreserveAspectFit//Image.Stretch
                mipmap: true
                asynchronous: true
                    
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
                        console.log("Loading product page ...");
                        pageLoader.setSource("qrc:/qml/pages/ProductPage.qml", { "model": modelData })
                    }
                }                    
            }
        }
            
        Image {
            id: verifiedPurchaseIcon
            source: "qrc:/images/paid.png"//neroshopResourceDir + "/paid.png"
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
                    property bool disabled: true
                    icon.source: "qrc:/images/heart.png"
                    icon.color: NeroshopComponents.Style.disabledColor//"#ffffff"
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
                        }
                        else {
                            disabled = true
                            icon.color = NeroshopComponents.Style.disabledColor
                        }
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
                        source: "qrc:/images/monero_symbol_white.png"
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
            Row { 
                id: starsRow
                Layout.alignment: (!settingsDialog.gridDetailsAlignCenter) ? 0 : Qt.AlignHCenter
                //spacing: 5
                property real avg_stars: Backend.getProductAverageStars(modelData.product_id)
                property int star_ratings_count: Backend.getProductStarCount(modelData.product_id)
                //Component.onCompleted: console.log("avg stars", starsRow.avg_stars)
                Repeater {
                    model: 5
                    delegate: Text {
                        text: (starsRow.star_ratings_count <= 0) ? qsTr(FontAwesome.star) : ((starsRow.avg_stars > index && starsRow.avg_stars < (index + 1)) ? qsTr(FontAwesome.starHalfStroke) : qsTr(FontAwesome.star)) // not sure?
                        color: ((index + 1) > Math.ceil(starsRow.avg_stars) || (starsRow.star_ratings_count <= 0)) ? "#777" : "#ffb344" // good!
                        font.bold: true
                        font.family: FontAwesome.fontFamily                            
                    }       
                }
                Text {
                    visible: !settingsDialog.hideProductDetails
                    text: qsTr(" %1"/* ratings"*/).arg((starsRow.star_ratings_count <= 0) ? "" : "(" + starsRow.star_ratings_count + ")")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                }
            }
        } // ColumnLayout
    } // delegate
} // Catalog View (grid)
