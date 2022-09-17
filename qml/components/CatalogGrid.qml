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
        id: catalog_grid
        rows: 2
        columns: 3
        spacing: 5//rowSpacing: 5; columnSpacing: 5
        //flow: Grid.TopToBottom
        function getBox(index) { // or get_item(index)?
            return catalog_grid_repeater.itemAt(index);
        }
        function getBoxCount() {
            return catalog_grid_repeater.count; // count is really just the number of items in the model :O
        }
        
        Repeater { // owns all items it instantiates
            id: catalog_grid_repeater
            model: (rows * columns)//fruitModel // rows and columns already set so this is useless (I think)
            // product box (GridBox)
            delegate: Rectangle { // delegates have a readonly "index" property that indicates the index of the delegate within the repeater
                id: product_box
                visible: true
                width: 220
                height: 220
                color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"// #a0a0a0 = 160,160,160
                //border.color: "white"
                //border.width: 1
                radius: 5
            
                Image {
                    id: verified_purchase_icon
                    source: "file:///" + neroshopResourcesDir + "/paid.png"//neroshopResourceDir + "/paid.png"
                    visible: true//false // only show this icon if item has been purchased previously
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 10
                
                    height: 24; width: 24 // has no effect since image is scaled
                    fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping            
                    
                    MouseArea {
                        id: verified_purchase_icon_mouse_area
                        anchors.fill: parent
                        hoverEnabled: true
                        //acceptedButtons: Qt.LeftButton // for heart_icon ONLY
                        onEntered: {
                            console.log("Mouse is over paid icon")
                            // show tooltip - todo: show tooltip while mouse is hovered over
                            ////if(has_purchased) {
                            console.log("Icon pos (" + catalog_grid_repeater.itemAt(index).children[0].x + ", " + catalog_grid_repeater.itemAt(index).children[0].y + ")")
                            console.log("Hint pos (" + hint.x + ", " + hint.y + ")")
                            console.log("Grid pos (" + catalog_grid.x + ", " + catalog_grid.y + ")")
                            console.log("Grid Parent pos (" + parent.x + ", " + parent.y + ")")
                            //hint.x = catalog_grid_repeater.itemAt(index).children[0].x//hint.anchors.left = catalog_grid_repeater.itemAt(index).left//catalog_grid_repeater.itemAt(0).horizontalCenter//.left//parent.x + ((parent.width - this.width) / 2)
                            //hint.y = catalog_grid_repeater.itemAt(index).children[0].y
                            //hint.x = catalog_grid.x + catalog_grid_repeater.itemAt(index).children[0].x// + catalog_grid_repeater.itemAt(index).children[0].width
                            //hint.y = catalog_grid.y + catalog_grid_repeater.itemAt(index).children[0].y
                            /*NeroshopComponents.Hint*/hint.show("You've previously purchased this item", -1)//(!is_favorited) ? "Add to favorites" : "Remove from favorites"
                            ////}
                        }   
                        onExited: {
                            hint.hide()
                        }         
                        //onClicked: { // for heart_icon ColorOverlay ONLY
                            //if(!is_favorited) heart_icon.color = "#808080"
                            //else heart_icon.color = "#e05d5d"
                        //}           
                    }                
                }
                ColorOverlay {
                    anchors.fill: verified_purchase_icon
                    source: verified_purchase_icon
                    color:"#808080"//#808080 = 128, 128, 128//#1e509b = active color //(has_purchased) ? color: "#1e509b" : "#808080"
                    visible: verified_purchase_icon.visible
                }            
            
                Image {
                    id: heart_icon
                    source: "file:///" + neroshopResourcesDir + "/heart.png"//neroshopResourceDir + "/heart.png"
                    visible: true
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 10                
                
                    height: 24; width: 24 // has no effect since image is scaled
                    fillMode:Image.PreserveAspectFit; // the image is scaled uniformly to fit button without cropping                
                }
                ColorOverlay {
                    anchors.fill: heart_icon
                    source: heart_icon
                    color:"#808080"//#808080 = 128, 128, 128//#e05d5d = active color //(is_favorited) ? color: "#e05d5d" : "#808080"
                    visible: heart_icon.visible
                }
                            
                Image {
                    id: product_image
                    source: "file:///" + neroshopResourcesDir + "/image_gallery.png"//neroshopResourceDir + "/image_gallery.png"
                    anchors.centerIn: parent
                
                    width: 128
                    height: 128
                    fillMode:Image.Stretch                
                }
                /*ColorOverlay {
                    anchors.fill: product_image
                    source: product_image
                    color:"#808080"//#808080 = 128, 128, 128
                }*/ 
                
                Label {
                    id: product_name_label
                    text: ""//name
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: product_image.bottom
                    anchors.topMargin: 10
                }           
            } // Catalog View Box (grid box)
        } // Repeater
    } // Catalog View (grid)
