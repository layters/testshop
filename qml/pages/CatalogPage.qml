// this page will display all products that have been searched for by the user in the search field
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "../components" as NeroshopComponents

Page {
    //id: catalog_page
    background: Rectangle {
        //visible: true
        color:"transparent" // fixes white edges on borders when grid box radius is set
    }
    
    NeroshopComponents.Hint {
        id: hint
    }

    NeroshopComponents.FilterBox {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
    }    
    
    // Text that displays current page results information
    Text {
        text: "Page " + (catalog_stack.currentIndex + 1) + " of " + catalog_stack.count
        font.bold: true
        x: 500
        y: 0
    }
        
    NeroshopComponents.ViewToggle {
        x: parent.x + (parent.width - this.width) / 2
        y: parent.y + 20
    }        

    // stackview functions
    function goToNextPage() {
        catalog_stack.currentIndex = catalog_stack.currentIndex + 1
        if(catalog_stack.currentIndex >= (catalog_stack.count - 1)) catalog_stack.currentIndex = (catalog_stack.count - 1)
    }
    function goToPrevPage() {
        catalog_stack.currentIndex = catalog_stack.currentIndex - 1
        if(catalog_stack.currentIndex <= 0) catalog_stack.currentIndex = 0
    }
    function setCurrentPageIndex(numberInput) {
         // if numberInput is greater than (count - 1) then equal it to (count - 1)
         if(Number(numberInput) >= (catalog_stack.count - 1)) {
             numberInput = (catalog_stack.count - 1)
         }
         // if numberInput is less than 0 then equal it to 0
         if(Number(numberInput) <= 0) {
             numberInput = 0
         }
         catalog_stack.currentIndex = numberInput
    }
    function getPageCount() { // Returns total number of grid pages belonging to the catalog StackLayout
        return catalog_stack.count;
    }
    function getCurrentPageIndex() {
        return catalog_stack.currentIndex;
    }
    function getItemsCount() {
        // ... boxesPerGrid * pageCount
    }
    // Pagination mode (Infinite Scroll mode replaces the StackLayout with a ScrollView)
    StackLayout {
        id: catalog_stack
        anchors.centerIn: parent
        currentIndex: 0
        width: NeroshopComponents.CatalogGrid.width; height: NeroshopComponents.CatalogGrid.height//width: pages.itemAt(this.currentIndex).width; height: pages.itemAt(this.currentIndex).height;//width: catalog.width; height: catalog.height

        Repeater {
            id: pages
            model: 10 // Number of page results from search
            delegate: NeroshopComponents.CatalogGrid {
            // To access each grid: pages.itemAt(index)
            // To access each box inside the grid: pages.itemAt(gridIndex).children[gridBoxIndex] //or: pages.itemAt(gridIndex).getBox(gridBoxIndex)
                //id: catalog
                // Items in a StackLayout support these attached properties (anchors cannot be used when managed by StackLayout or any other Layout type):
                //Layout.minimumWidth
                //Layout.minimumHeight
                //Layout.preferredWidth
                //Layout.preferredHeight
                //Layout.maximumWidth
                //Layout.maximumHeight
                //Layout.fillWidth: true
                //Layout.fillHeight: true                
            }
        }
    } // StackLayout
    // Custom pagination bar
    NeroshopComponents.PaginationBar {
        id: pagination
        firstButton.onClicked: { if(!firstButton.disabled) goToPrevPage() }
        secondButton.onClicked: { if(!secondButton.disabled) goToNextPage() }
        numberField.onEditingFinished: { setCurrentPageIndex(numberField.text - 1) }
        currentIndex: catalog_stack.currentIndex
        count: catalog_stack.count
        // For Row ONLY, NOT RowLayout (use Layout.alignment for RowLayout instead)
        anchors.left: catalog_stack.left
        anchors.leftMargin: (catalog_stack.width - this.width) / 2
        anchors.top: catalog_stack.bottom
        anchors.topMargin: 20
    }                             
}
