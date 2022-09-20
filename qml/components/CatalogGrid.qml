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
        rows: 2
        columns: 3
        spacing: 5//rowSpacing: 5; columnSpacing: 5
        //flow: Grid.TopToBottom
        function getBox(index) { // or get_item(index)?
            return catalogGridRepeater.itemAt(index);
        }
        function getBoxCount() {
            return catalogGridRepeater.count; // count is really just the number of items in the model :O
        }
        
        Repeater { // owns all items it instantiates
            id: catalogGridRepeater
            model: (rows * columns)//fruitModel // rows and columns already set so this is useless (I think)
            // product box (GridBox)
            delegate: Rectangle { // delegates have a readonly "index" property that indicates the index of the delegate within the repeater
                id: product_box
                visible: true
                width: 220
                height: 220
                color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"//"#ffffff"// #a0a0a0 = 160,160,160
                //border.color: "white"
                //border.width: 1
                radius: 5
            
                Image {
                    id: verifiedPurchaseIcon
                    source: "file:///" + neroshopResourcesDir + "/paid.png"//neroshopResourceDir + "/paid.png"
                    visible: false // only show this icon if item has been purchased previously
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 10
                
                    height: 24; width: 24 // has no effect since image is scaled
                    fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping            
                    mipmap: true
                    
                    MouseArea {
                        id: verifiedPurchaseIconMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            ////if(has_purchased) { // if item has been purchased previously then this icon will be visible, so no need for this
                            let boxPositionInWindow = mapToItem(mainWindow.contentItem, verifiedPurchaseIconMouseArea.x, verifiedPurchaseIconMouseArea.y)
                            let box = catalogGridRepeater.itemAt(index).children[0]
                            catalogHint.x = boxPositionInWindow.x + (box.width - catalogHint.width) / 2
                            catalogHint.y = boxPositionInWindow.y + box.height + 5
                            catalogHint.show("You've previously purchased this item", 3000) // hide after 3 seconds
                            ////}
                        }   
                        onExited: {
                            catalogHint.hide()
                        }
                    }                
                }
                ColorOverlay {
                    anchors.fill: verifiedPurchaseIcon
                    source: verifiedPurchaseIcon
                    color: (NeroshopComponents.Style.darkTheme) ? "#6699cc" : "#1e509b"//"#336699"// // activeColor
                    visible: verifiedPurchaseIcon.visible
                }            
            
                Image {
                    id: heartIcon
                    source: "file:///" + neroshopResourcesDir + "/heart.png"//neroshopResourceDir + "/heart.png"
                    visible: true
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 10                
                
                    height: 24; width: 24 // has no effect since image is scaled
                    fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping
                    mipmap: true
                    
                    MouseArea {
                        id: heartIconMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onEntered: {
                            let boxPositionInWindow = mapToItem(mainWindow.contentItem, heartIconMouseArea.x, heartIconMouseArea.y)
                            let box = catalogGridRepeater.itemAt(index).children[0]
                            catalogHint.x = boxPositionInWindow.x + (box.width - catalogHint.width) / 2
                            catalogHint.y = boxPositionInWindow.y + box.height + 5
                            catalogHint.show("Add to favorites", 3000)////(!is_favorited) ? catalogHint.show("Add to favorites", -1) : catalogHint.show("Remove from favorites")
                        }   
                        onExited: {
                            catalogHint.hide()
                        }         
                        onClicked: { 
                            /*if(!is_favorited) heartIconOverlay.color = "#808080"
                            else heartIconOverlay.color = "#e05d5d"*/
                            heartIconOverlay.color = "#e05d5d"
                        }           
                    }                                    
                }
                ColorOverlay {
                    id: heartIconOverlay
                    anchors.fill: heartIcon
                    source: heartIcon
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : NeroshopComponents.Style.disabledColor//#e05d5d = active color ////(is_favorited) ? color: "#e05d5d" : "#808080"
                    visible: heartIcon.visible
                }
                            
                Image {
                    id: productImage
                    source: "file:///" + neroshopResourcesDir + "/image_gallery.png"//neroshopResourceDir + "/image_gallery.png"
                    anchors.centerIn: parent
                
                    width: 128
                    height: 128
                    fillMode:Image.Stretch
                    //mipmap: true
                }
                /*ColorOverlay {
                    anchors.fill: productImage
                    source: productImage
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : NeroshopComponents.Style.disabledColor
                    visible: productImage.visible
                }*/
                
                Label {
                    id: productNameLabel
                    text: ""//name
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: productImage.bottom
                    anchors.topMargin: 10
                }           
            } // Catalog View Box (grid box)
        } // Repeater
    } // Catalog View (grid)
