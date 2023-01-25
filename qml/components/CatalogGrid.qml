// this page will display all products that have been searched for by the user in the search field
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "." as NeroshopComponents
    // todo: place grid in scrollview (infinite scroll mode) but in a separate file called CatalogGridViewInfiniteScroll.qml or CatalogGridViewLimitlessScroll.qml
    // todo: create a CatalogList.qml for the Catalog List View
    // Grid should have two modes: Pagination mode and Infinite scroll mode
    // catalog view (Grid)
    Grid {
        id: catalogGrid
        rows: 10//20
        columns: 3
        spacing: 5//rowSpacing: 5; columnSpacing: 5
        //flow: Grid.TopToBottom
        function getBox(index) { // or get_item(index)?
            return catalogGridRepeater.itemAt(index);
        }
        function getBoxCount() {
            return catalogGridRepeater.count; // count is really just the number of items in the model :O
        }
        property bool hideProductDetails: false // hides product name, price, and star ratings if set to true
        property real boxWidth: (hideProductDetails) ? 250 : 300//220 : 250//300
        property real boxHeight: 300//(hideProductDetails) ? 300 : 400
        property real fullWidth: (this.boxWidth * columns) + (spacing * (columns - 1)) // Full width of the entire grid
        property alias count: catalogGridRepeater.count
        
        Repeater { // owns all items it instantiates
            id: catalogGridRepeater
            model: Backend.getListings()//(rows * columns)//// rows and columns already set so this is useless (I think)
            // product box (GridBox)
            delegate: Rectangle { // delegates have a readonly "index" property that indicates the index of the delegate within the repeater
                id: productBox
                visible: true
                width: catalogGrid.boxWidth
                height: catalogGrid.boxHeight
                color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"//"#ffffff"// #a0a0a0 = 160,160,160
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                border.width: 0
                radius: 5
                clip: true // So that productNameText will be clipped to the Rectangle's bounding rectangle and will not go past it
                            
                Rectangle {
                    id: productImageRect
                    anchors.top: parent.top//; anchors.topMargin: parent.border.width//0//5
                    anchors.left: parent.left // so that margins will also apply to left and right sides
                    anchors.margins: parent.border.width
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width; height: (parent.height / 3) + 30//=130
                    color: "#ffffff"//"transparent"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    radius: parent.radius
                             
                    Image {
                        id: productImage
                        source: "file:///" + modelData.product_image_file//"qrc:/images/image_gallery.png"
                        anchors.centerIn: parent
                        width: 128; height: 128
                        fillMode: Image.PreserveAspectFit//Image.Stretch
                        mipmap: true
                        asynchronous: true
                    
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            onEntered: {
                                catalogGridRepeater.itemAt(index).border.width = 1
                            }
                            onExited: {
                                catalogGridRepeater.itemAt(index).border.width = 0
                            }
                            onClicked: { 
                                navBar.uncheckAllButtons() // Uncheck all navigational buttons
                                console.log("Loading product page ...");
                                pageLoader.setSource("qrc:/qml/pages/ProductPage.qml")////, { "listingId": modelData.listing_uuid })
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
                // todo: maybe use a Flow for text wrapping? // Reminder: Flow children cannot have anchors or positions set!!
                TextArea {
                    id: productNameText
                    anchors.left: parent.left
                    anchors.top: productImageRect.bottom
                    anchors.right: parent.right
                    anchors.margins: 10
                    text: qsTr(modelData.product_name)//qsTr("Product name")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    visible: !catalogGrid.hideProductDetails
                    readOnly: true
                    wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width
                    selectByMouse: true
                    //font.bold: true
                    font.pointSize: 13
                    background: Rectangle { color: "transparent" }
                    padding: 0; leftPadding: 0
                }
                                
                Item {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: productNameText.bottom//priceFiat.bottom
                    anchors.topMargin: 5////10
                    
                    Column {
                        spacing: 5//10
                        Row {
                            spacing: 5
                        
                            Rectangle {
                                id: moneroSymbolRect
                                radius: 50
                                color: "transparent"
                                border.color: NeroshopComponents.Style.moneroOrangeColor
                                width: moneroSymbol.width + 2; height: moneroSymbol.height + 2
                                
                                Image {
                                    id: moneroSymbol
                                    source: "qrc:/images/monero_symbol.png"
                                    visible: !catalogGrid.hideProductDetails
                                    width: 24; height: 24
                                    fillMode:Image.PreserveAspectFit
                                    mipmap: true
                                    anchors.verticalCenter: parent.verticalCenter; anchors.horizontalCenter: parent.horizontalCenter//anchors.centerIn: parent//verticalAlignment: Image.AlignVCenter; horizontalAlignment: Image.AlignHCenter
                                }
                            }
                                
                            TextField { // Allows us to copy the string unlike Text
                                id: priceMonero
                                text: qsTr("%1").arg(Backend.convertToXmr(Number(modelData.price), modelData.currency).toFixed(Backend.getCurrencyDecimals("XMR")))//.arg("XMR") // TODO: allow users to specificy their preferred number of digits
                                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                visible: !catalogGrid.hideProductDetails
                                anchors.verticalCenter: moneroSymbolRect.verticalCenter
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
                            text: qsTr("%1%2 %3").arg(Backend.getCurrencySign(modelData.currency)).arg(Number(modelData.price).toFixed(Backend.getCurrencyDecimals(modelData.currency))).arg(modelData.currency)
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            visible: !catalogGrid.hideProductDetails
                            readOnly: true
                            selectByMouse: true
                            background: Rectangle {
                                color: "transparent"
                            }
                            padding: 0; leftPadding: 0 // With the padding at zero, we can freely set the Row spacing
                        } 
                    }
                }
                // TODO: Ratings stars and ratings count
            } // Catalog View Box (grid box)
        } // Repeater
    } // Catalog View (grid)
