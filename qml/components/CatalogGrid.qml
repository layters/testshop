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
        rows: 20//2
        columns: 3
        spacing: 5//rowSpacing: 5; columnSpacing: 5
        //flow: Grid.TopToBottom
        function getBox(index) { // or get_item(index)?
            return catalogGridRepeater.itemAt(index);
        }
        function getBoxCount() {
            return catalogGridRepeater.count; // count is really just the number of items in the model :O
        }
        property bool hideProductDetails: true//false // hides product name, price, and star ratings if set to true
        Repeater { // owns all items it instantiates
            id: catalogGridRepeater
            model: (rows * columns)// rows and columns already set so this is useless (I think)
            // product box (GridBox)
            delegate: Rectangle { // delegates have a readonly "index" property that indicates the index of the delegate within the repeater
                id: product_box
                visible: true
                width: (hideProductDetails) ? 250 : 300//220 : 250//300
                height: (hideProductDetails) ? 300 : 400//220 : 250//220
                color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"//"#ffffff"// #a0a0a0 = 160,160,160
                border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                border.width: 0
                radius: 5
                clip: true // So that productNameLabel will be clipped to the Rectangle's bounding rectangle and will not go past it
                            
                Image {
                    id: productImage
                    source: "qrc:/images/image_gallery.png"
                    anchors.horizontalCenter: parent.horizontalCenter//anchors.centerIn: parent
                    anchors.top: heartIconButton.top//parent.top
                    anchors.topMargin: 20
                
                    width: 128
                    height: 128
                    fillMode:Image.Stretch
                    mipmap: true
                    
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
                            ////pageLoader.source = "../pages/ProductPage.qml"
                        }
                    }                    
                }
                /*ColorOverlay {
                    anchors.fill: productImage
                    source: productImage
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : NeroshopComponents.Style.disabledColor
                    visible: productImage.visible
                }*/
            
                Image {
                    id: verifiedPurchaseIcon
                    source: "qrc:/images/paid.png"//neroshopResourceDir + "/paid.png"
                    visible: false // only show this icon if item has been purchased previously
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
                    color: (NeroshopComponents.Style.darkTheme) ? "#6699cc" : "#1e509b"//"#336699"// // activeColor
                    visible: verifiedPurchaseIcon.visible
                }            
                                
                Button { // heartIconButton must be drawn over the productImage
                    id: heartIconButton
                    text: qsTr("Add to favorites")
                    display: AbstractButton.IconOnly // will only show the icon and not the text
                    hoverEnabled: true
                    ////containmentMask: this.icon // When the mouse is pointing at the icon's bounding box instead of the button's then the border will appear
                    //width: 24; height: 24
                    anchors.right: parent.right
                    anchors.rightMargin: 5//10
                    anchors.top: parent.top
                    anchors.topMargin: 5//10
                    
                    icon.source: "qrc:/images/heart.png"
                    icon.color: "#ffffff"
                    icon.height: 24; icon.width: 24
                    
                    background: Rectangle {
                        color: "transparent"
                        radius: 3//0
                        border.color: {
                            if(NeroshopComponents.Style.darkTheme && parent.hovered) {
                                return "#ffffff"
                            }
                            else if(!NeroshopComponents.Style.darkTheme && parent.hovered) {
                                return "#000000"
                            }
                            else return "transparent"
                        }//parent.hovered ? "#ffffff" : "transparent"
                    }    
                    
                    NeroshopComponents.Hint {
                        id: heartIconButtonHint
                        visible: parent.hovered
                        text: parent.text
                        pointer.visible: false
                        delay: 500; timeout: 3000
                    }
                    
                    onClicked: { 
                        icon.color = "#e05d5d"
                    }
                }                
                // todo: maybe use a Flow for text wrapping? // Reminder: Flow children cannot have anchors or positions set!!
                Label {
                    id: productNameLabel
                    text: qsTr("A really extremely long as product name that can probably not fit the entire grid")//qsTr("Product name")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    visible: !catalogGrid.hideProductDetails
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: productImage.bottom
                    anchors.topMargin: 10
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                }
                
                Label {
                    id: priceFiat
                    text: qsTr("%1 %2 %3").arg("$").arg("0.00").arg("USD")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    visible: !catalogGrid.hideProductDetails
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: productNameLabel.bottom
                    anchors.topMargin: 10
                } 
                
                Image {
                    id: moneroSymbol
                    source: "qrc:/images/monero_symbol.png"//"/monero_symbol_white.png"
                    visible: !catalogGrid.hideProductDetails
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: priceFiat.bottom
                    anchors.topMargin: 5//10
                    width: 16; height: 16
                    fillMode:Image.PreserveAspectFit
                    mipmap: true
                }
                                
                Label {
                    id: priceMonero
                    text: qsTr("0.000000000000 %1").arg("XMR")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    visible: !catalogGrid.hideProductDetails
                    anchors.left: moneroSymbol.right
                    anchors.leftMargin: 5
                    anchors.verticalCenter: moneroSymbol.verticalCenter
                }
 
 states: [
   State {
     name: "wide text"
     when: parent.children[4].contentWidth > parent.width//.children[4].text.length > 20
     PropertyChanges {
         target: parent.children[4]; 
         width: 5000
         height: parent.children[4].paintedHeight
     }
   }/*,
   State {
     name: "not wide text"
     when: catalogGridRepeater.itemAt(index).children[4].text.length <= 20
     PropertyChanges {
         target: catalogGridRepeater.itemAt(index).children[4]; 
         width: dummy_text.paintedWidth
         height: catalogGridRepeater.itemAt(index).children[4].paintedHeight
     }*/
 ]                                                                       
            } // Catalog View Box (grid box)
        } // Repeater
    } // Catalog View (grid)
