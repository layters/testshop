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
        catalogScrollView.ScrollBar.vertical.position = 0.0//console.log("Scrollbar reset")
    }
    function goToNextPage() {
        catalogStack.currentPageIndex = catalogStack.currentPageIndex + 1
        if(catalogStack.currentPageIndex >= (catalogStack.pages.count - 1)) catalogStack.currentPageIndex = (catalogStack.pages.count - 1)
        resetScrollBar()
    }
    function goToPrevPage() {
        catalogStack.currentPageIndex = catalogStack.currentPageIndex - 1
        if(catalogStack.currentPageIndex <= 0) catalogStack.currentPageIndex = 0
        resetScrollBar()
    }
    function setCurrentPageIndex(numberInput) {
         // if numberInput is greater than (count - 1) then equal it to (count - 1)
         if(Number(numberInput) >= (catalogStack.pages.count - 1)) {
             numberInput = (catalogStack.pages.count - 1)
         }
         // if numberInput is less than 0 then equal it to 0
         if(Number(numberInput) <= 0) {
             numberInput = 0
         }
         catalogStack.currentPageIndex = numberInput
         ////resetScrollBar()
    }
    function getPageCount() { // Returns total number of grid pages belonging to the catalog StackLayout
        return catalogStack.pages.count;
    }
    function getCurrentPageIndex() {
        return catalogStack.pages.currentIndex;
    }
    function getItemsCount() {
        // ... boxesPerGrid * pageCount
    }    
    
    ScrollView {
        id: catalogScrollView
        /*background: Rectangle {
            color: "transparent" // fixes white edges on borders when grid box radius is set
        }*/
        anchors.fill: parent
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn//ScrollBar.AsNeeded
        clip: true // The area in which the contents of the filterBox will be bounded to (set width and height) // If clip is false then the contents will go beyond/outside of the filterBox's bounds
        //contentWidth: mainWindow.width//catalogStack.width
        contentHeight: childrenRect.height + 200////catalogStack.height + 200//100 for viewToggle space and 100 for pagination space //mainWindow.height

        NeroshopComponents.FilterBox {
            id: productFilterBox
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 20
        }    

        // Text that displays current page results information
        /*Text {
            id: pageResultsDisplay
            text: "Page " + (catalogStack.pages.currentIndex + 1) + " of " + catalogStack.pages.count//text: "Page " + (catalogStack.currentIndex + 1) + " of " + catalogStack.count
            font.bold: true
            anchors.left: catalogStack.left
            anchors.top: viewToggle.top//0
            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
        }*/
                
        
        NeroshopComponents.ViewToggle {
            id: viewToggle
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
        }             

        // Pagination mode (Infinite Scroll mode replaces the StackLayout with a ScrollView)
        StackLayout {
            id: catalogStack
            width: parent.width; height: childrenRect.height
            anchors.horizontalCenter: viewToggle.horizontalCenter
            anchors.left: (productFilterBox.visible) ? productFilterBox.right : parent.left
            anchors.leftMargin: (productFilterBox.visible) ? 10 : 20
            anchors.top: viewToggle.bottom
            anchors.topMargin: 50
            currentIndex: viewToggle.currentIndex
            property int pagesCount: 1//10////pages.count
            property var currentItem: this.itemAt(this.currentIndex)
            property var pages: currentItem.children[0]
            property int currentPageIndex: 0
            property var pageItem: pages.children[0] // can be either a CatalogGrid or CatalogList
            // Infinite Scroll Mode (without pages)
            Item {
                id: gridItem
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                StackLayout {
                    anchors.fill: parent
                    currentIndex: catalogStack.currentPageIndex
                    Repeater {
                        model: catalogStack.pagesCount//1
                        delegate: NeroshopComponents.CatalogGrid { // has index property
                        }
                    }
                }
            }
            
            Item {
                id: listItem
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                StackLayout {
                    anchors.fill: parent
                    currentIndex: catalogStack.currentPageIndex
                    Repeater {
                        model: catalogStack.pagesCount//1
                        delegate: NeroshopComponents.CatalogList {
                        }
                    }
                }
            }
            // Pagination Mode (with pages)
        /*Repeater {
            id: pages
            model: 1//10 // Number of page results from search
            // todo: use a Loader to switch between grid view and list view // https://forum.qt.io/topic/80826/dynamic-delegate-in-repeater/4 //// (viewToggle.checkedButton.text.match("List view")) ? 
            delegate: Loader {////NeroshopComponents.CatalogGrid {
                id: catalogViewLoader
                source: (viewToggle.currentIndex == 0) ? "qrc:/qml/components/CatalogGrid.qml" : "qrc:/qml/components/CatalogList.qml"*/
                //asynchronous: true // causes Segfault
                // displaying the catalog grid's properties
                //onLoaded: {
                    /*console.log("Full width of Grid (actual):", catalogViewLoader.item.width)
                    console.log("catalogGrid.fullWidth:", catalogViewLoader.item.fullWidth)*/
                    //console.log(catalogViewLoader.item.rows)
                    //console.log("model count:", catalogViewLoader.item.count)
                //}
                // Changing the catalog grid's properties
                /*Component.onCompleted: {
                    catalogViewLoader.setSource(catalogViewLoader.source,
                             { "rows": "2" });
                }*/
            /*}
        }*/
        } // StackLayout
        // Custom pagination bar
        NeroshopComponents.PaginationBar {
            id: pagination
            firstButton.onClicked: { if(!firstButton.disabled) goToPrevPage() }
            secondButton.onClicked: { if(!secondButton.disabled) goToNextPage() }
            numberField.onEditingFinished: { setCurrentPageIndex(numberField.text - 1) }
            currentIndex: catalogStack.pages.currentIndex//catalogStack.currentIndex
            count: catalogStack.pages.count//catalogStack.count
            // For Row ONLY, NOT RowLayout (use Layout.alignment for RowLayout instead)
            anchors.horizontalCenter: parent.horizontalCenter//anchors.horizontalCenter: catalogStack.horizontalCenter
            anchors.bottom: parent.bottom; anchors.bottomMargin: 20
        }         
    } // ScrollView or Flickable
} // Page
