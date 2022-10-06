// this page will display all products that have been searched for by the user in the search field
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "../components" as NeroshopComponents

Page {
    id: catalogPage
    background: Rectangle {
        color: "transparent"
    }
    function resetScrollBar() {
        catalogScrollView.ScrollBar.vertical.position = 0.0
        console.log("Scrollbar reset")
    }
    function goToNextPage() {
        catalogStack.currentIndex = catalogStack.currentIndex + 1
        if(catalogStack.currentIndex >= (catalogStack.count - 1)) catalogStack.currentIndex = (catalogStack.count - 1)
        resetScrollBar()
    }
    function goToPrevPage() {
        catalogStack.currentIndex = catalogStack.currentIndex - 1
        if(catalogStack.currentIndex <= 0) catalogStack.currentIndex = 0
        resetScrollBar()
    }
    function setCurrentPageIndex(numberInput) {
         // if numberInput is greater than (count - 1) then equal it to (count - 1)
         if(Number(numberInput) >= (catalogStack.count - 1)) {
             numberInput = (catalogStack.count - 1)
         }
         // if numberInput is less than 0 then equal it to 0
         if(Number(numberInput) <= 0) {
             numberInput = 0
         }
         catalogStack.currentIndex = numberInput
         ////resetScrollBar()
    }
    function getPageCount() { // Returns total number of grid pages belonging to the catalog StackLayout
        return catalogStack.count;
    }
    function getCurrentPageIndex() {
        return catalogStack.currentIndex;
    }
    function getItemsCount() {
        // ... boxesPerGrid * pageCount
    }    
ScrollView {//Flickable { 
    id: catalogScrollView
    background: Rectangle {
        color: "transparent" // fixes white edges on borders when grid box radius is set
    }
    anchors.fill: parent
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn//ScrollBar.AsNeeded//ScrollBar.AlwaysOn        
    clip: true // The area in which the contents of the filterBox will be bounded to (set width and height) // If clip is false then the contents will go beyond/outside of the filterBox's bounds
    //contentWidth: mainWindow.width//catalogStack.width
    contentHeight: catalogStack.height + 200//100 for viewToggle space and 100 for pagination space //mainWindow.height

    NeroshopComponents.ViewToggle {
        id: viewToggle
        anchors.horizontalCenter: catalogStack.horizontalCenter//parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
    }   
            
    NeroshopComponents.Hint {
        id: catalogHint
        pointer.visible: false
        delay: 500
    }

    NeroshopComponents.FilterBox {
        id: productFilterBox
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
    }    
    
    // Text that displays current page results information
    Text {
        id: pageResultsDisplay
        text: "Page " + (catalogStack.currentIndex + 1) + " of " + catalogStack.count
        font.bold: true
        anchors.left: catalogStack.left
        anchors.top: viewToggle.top//0
        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
    }
             

    // Pagination mode (Infinite Scroll mode replaces the StackLayout with a ScrollView)
    StackLayout {
        id: catalogStack
        //anchors.horizontalCenter: viewToggle.horizontalCenter
        anchors.left: (productFilterBox.visible) ? productFilterBox.right : parent.left
        anchors.leftMargin: (productFilterBox.visible) ? 10 : 20
        anchors.top: viewToggle.bottom
        anchors.topMargin: 50
        currentIndex: 0
        width: NeroshopComponents.CatalogGrid.width; height: NeroshopComponents.CatalogGrid.height//width: pages.itemAt(this.currentIndex).width; height: pages.itemAt(this.currentIndex).height;//width: catalog.width; height: catalog.height

        Repeater {
            id: pages
            model: 10 // Number of page results from search
            // todo: use a Loader to switch between grid view and list view // https://forum.qt.io/topic/80826/dynamic-delegate-in-repeater/4 //// (viewToggle.checkedButton.text.match("List view")) ? 
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
        currentIndex: catalogStack.currentIndex
        count: catalogStack.count
        // For Row ONLY, NOT RowLayout (use Layout.alignment for RowLayout instead)
        anchors.horizontalCenter: catalogStack.horizontalCenter
        anchors.top: catalogStack.bottom
        anchors.topMargin: 20
    }         
    // This is not necessaary unless you are using a Flickable
            /*ScrollBar.vertical: ScrollBar {
            //width: 20
            policy: ScrollBar.AlwaysOn//ScrollBar.AsNeeded//ScrollBar.AlwaysOn
        }*/ // Scrollbar                    
} // ScrollView or Flickable
} // Page
