// this page will display all products that have been searched for by the user in the search field
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "../components" as NeroshopComponents

Page {
    id: catalog_page
    background: Rectangle {
        //visible: true
        color:"transparent" // fixes white edges on borders when grid box radius is set
    }
    // test button
Button {
    id: test_button
    text: "Test"
    onClicked: {
        /*if(catalog_view.rows == (3 * catalog_pages.index)) {
            console.log("3 rows reached. Move to next page");
            return;
        }*/
        // push 3 new items
        // repeater will insert the items since its model is fruitModel
        //fruitModel.insert(0, {"cost": 5.95, "name":"Pizza"})
        //fruitModel.insert(0, {"cost": 7.99, "name":"Cake"})
        //fruitModel.insert(0, {"cost": 2.15, "name":"Soda"})
        //fruitModel.insert(2, {"cost": 2.88, "name":"Chips"})
        console.log("number of models in fruitModel: " + fruitModel.count)
        //catalog_grid_repeater.itemAt(0).name = "DUDE"
        // This works
        console.log("Grid Boxes count: " + catalog.get_box_count())
        var box = catalog.get_box(3)//catalog_grid_repeater.itemAt(2)
        box.color = "blue"  
        console.log( catalog.get_box(3).children[0] ) // Image (verified purchase icon)      
        console.log( catalog.get_box(3).children[1] ) // ColorOverlay
        console.log( catalog.get_box(3).children[2] ) // Image (heart icon)
        console.log( catalog.get_box(3).children[3] ) // ColorOverlay
        console.log( catalog.get_box(3).children[4] ) // Image (product image)
        console.log( catalog.get_box(3).children[5] ) // Label (product name)
        console.log( catalog.get_box(3).children[6] ) // Label (product cost)
        //console.log( catalog.get_box(3).children[7] ) // ? (product stars)
        var box_index = 1
        var box_item = catalog.get_box(box_index);
        console.log("Catalog position: " + catalog.x + ", " + catalog.y )
        console.log("Box position: " + box_item.x + ", " + box_item.y )
        console.log("Box image(verified_icon) position: " + box_item.children[0].x + ", " + box_item.children[0].y )
    
        console.log("Hint width: " + hint.width)
        console.log("Catalog Page Pos: " + catalog_page.x + ", " + catalog_page.y )
        console.log("Catalog Page Size " + catalog_page.width + ", " + catalog_page.height )
        // push page to catalog_page_stack
        catalog_page_stack.push(NeroshopComponents.CatalogGrid, {})//, {"color": "red"})
        
    }
    x:-500
    background: Rectangle {
        color: "#6b5b95" // #ff6600 is the monero orange color
        radius: 0
    }    
}    
    // shapes
/*Shape {
    width: 200
    height: 150
    anchors.centerIn: parent
    ShapePath {
        strokeWidth: 4
        strokeColor: "red"
        strokeStyle: ShapePath.DashLine
        dashPattern: [ 1, 4 ]
        startX: 20; startY: 20
        PathLine { x: 180; y: 130 }
        PathLine { x: 20; y: 130 }
        PathLine { x: 20; y: 20 }
    }
}
*/    
// list model
ListModel { // import QtQml.Models 2.15
    id: fruitModel

    ListElement {
        name: "Apple"
        cost: 2.45
        attributes: [
            ListElement { description: "Core" },
            ListElement { description: "Deciduous" }
        ]
    }
    ListElement {
        name: "Orange"
        cost: 3.25
        attributes: [
            ListElement { description: "Citrus" }
        ]        
    }
    ListElement {
        name: "Banana"
        cost: 1.95
        attributes: [
            ListElement { description: "Tropical" },
            ListElement { description: "Seedless" }
        ]        
    }
    /*ListElement {
        name: "Watermelon"
        cost: 3.95
        attributes: [
            ListElement { description: "Tropical" },
            ListElement { description: "Natural/Organic" }
        ]        
    }*/    
}

NeroshopComponents.Hint {
    id: hint
}
    // navigational buttons
    /*RowLayout {
    }*/
    // todo: place grid in stackview or swipeview for multiple grid pages (pagination mode)
    // todo: place grid in scrollview (infinite scroll mode) but in a separate file called CatalogGridViewInfiniteScroll.qml
    // todo: move this code to CatalogGridView.qml
    // todo: create a CatalogListView
    // stackview functions
    ////function next_page() {}
    ////function prev_page() {}
// Pagination mode
StackView {
    id: catalog_page_stack
    anchors.fill: parent
    //index: 1//currentIndex: 1
    // Repeater inside StackView?
// Infinite scroll mode
/*Flickable {
    anchors.fill: parent
    contentHeight: catalog_view.height
    contentWidth: catalog_view.width*/
    // catalog view (Grid)
    /*Grid {
        id: catalog_view
        rows: 2//40//2
        columns: 3
        spacing: 5//rowSpacing: 5; columnSpacing: 5
        anchors.centerIn: parent // place at center of parent
        //flow: Grid.TopToBottom
        
        Repeater { // owns all items it instantiates
            id: catalog_grid_repeater
            model: fruitModel//10 // rows and columns already set so this is useless
            //delegate: this { id: _id }
            // product box (GridBox)
            Rectangle {
                id: product_box
                visible: true
                width: 220
                height: 220
                color: "#a0a0a0"// #a0a0a0 = 160,160,160
                //border.color: "white"
                //border.width: 1
                radius: 5
            
                Image {
                    id: verified_purchase_icon
                    source: neroshopResourceDir + "/paid.png"
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
                            //hint.anchors.left = catalog_grid_repeater.itemAt(0).left//catalog_grid_repeater.itemAt(0).horizontalCenter//.left//parent.x + ((parent.width - this.width) / 2)
                            hint.show("You've previously purchased this item", -1)//(!is_favorited) ? "Add to favorites" : "Remove from favorites"
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
                    source: neroshopResourceDir + "/heart.png"
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
                    source: neroshopResourceDir + "/image_gallery.png"
                    anchors.centerIn: parent
                
                    width: 128
                    height: 128
                    fillMode:Image.Stretch                
                }
                //ColorOverlay {
                //    anchors.fill: product_image
                //    source: product_image
                //    color:"#808080"//#808080 = 128, 128, 128
                //} 
                
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
    }*/ // Catalog View (grid)
    //}
    NeroshopComponents.CatalogGrid {
        id: catalog
    }

                /*Button {
                    text: "<"
                    onClicked: catalog_page_stack.replace(page, StackView.PopTransition)
                }
                Button {
                    text: ">"
                    onClicked: catalog_page_stack.replace(page, StackView.PushTransition)
                }*/    
    
    } // StackView
}    
