// this page will display all products that have been searched for by the user in the search field
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import neroshop.Enums 1.0

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
        return catalogPage.model.length;// ... boxesPerGrid * pageCount
    }    
    //property alias catalogIndex: catalogStack.currentIndex
    property var model: null

        Rectangle {
            id: topPanel
            anchors.top: parent.top; anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            width: catalogStack.width; height: childrenRect.height
            color: "transparent"
            //border.color: "#ffffff"
            // Text that displays current page results information
            Text {
                id: pageResultsDisplay
                text: qsTr("Page %1 of %2 (Results: %3)").arg(catalogStack.pages.currentIndex + 1).arg(catalogStack.pages.count).arg(catalogPage.model.length)
                font.bold: true
                anchors.left: parent.left
                anchors.verticalCenter: viewToggle.verticalCenter//anchors.top: viewToggle.top
                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            }
            // ViewToggle
            NeroshopComponents.ViewToggle {
                id: viewToggle
                anchors.horizontalCenter: parent.horizontalCenter
                //anchors.top: parent.top; anchors.topMargin: 20
            }    
            //GroupBox {
            //        title: qsTr("Sort")
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: viewToggle.verticalCenter
                spacing: 3
                // Filter button
                Button {
                    id: filterButton
                    text: qsTr("Filter")
                    //anchors.verticalCenter: parent.verticalCenter
                    //height: sortBox.height
                    checkable: true
                    checked: filterPopUp.visible
                    display: AbstractButton.IconOnly
                    icon.source: "qrc:/assets/images/filter.png"
                    icon.color: !this.checked ? "#605185" : "#ffffff"
                    background: Rectangle {
                        radius: 3
                        color: parent.checked ? "#605185" : "#e0e0e0"
                    }
                    onClicked: {
                        filterPopUp.visible = true
                    }
                
                    Popup {
                        id: filterPopUp
                        visible: false
                        x: (parent.width - width) / 2
                        y: parent.height + 1
                        width: 200
                        height: 300//implicitHeight: contentItem.implicitHeight
                        
                        background: Rectangle {
                            radius: 5
                            color: "#e0e0e0"
                        }
                        
                        contentItem: /*NeroshopComponents.FilterBox {*/Item {
                           Text {
                               text: qsTr("Coming soon!")
                               anchors.centerIn: parent
                           }
                        }
                    }
                }
                // SortComboBox
                NeroshopComponents.ComboBox {
                    id: sortBox
                    //anchors.verticalCenter: parent.verticalCenter
                    width: 250; height: viewToggle.height
                    model: ["None", "Latest", "Oldest", "Alphabetical order", "Price - Lowest", "Price - Highest"]
                    Component.onCompleted: currentIndex = find("None")
                    displayText: "Sort: " + currentText
                    indicatorDoNotPassBorder: true
                    onActivated: {
                        if(currentIndex == find("None")) {
                            catalogPage.model = Backend.sortBy(catalogPage.model, Enum.Sorting.SortByOldest)//Enum.Sorting.SortNone)
                            settingsDialog.lastUsedListingSorting = Enum.Sorting.SortNone
                        }
                        if(currentIndex == find("Oldest")) {
                            catalogPage.model = Backend.sortBy(catalogPage.model, Enum.Sorting.SortByOldest)
                            settingsDialog.lastUsedListingSorting = Enum.Sorting.SortByOldest
                        }
                        if(currentIndex == find("Latest")) {
                            console.log("Showing most recent items")
                            catalogPage.model = Backend.sortBy(catalogPage.model, Enum.Sorting.SortByMostRecent)
                            settingsDialog.lastUsedListingSorting = Enum.Sorting.SortByMostRecent
                        }
                        if(currentIndex == find("Alphabetical order")) {
                            catalogPage.model = Backend.sortBy(catalogPage.model, Enum.Sorting.SortByAlphabeticalOrder)
                            settingsDialog.lastUsedListingSorting = Enum.Sorting.SortByAlphabeticalOrder
                        }
                        if(currentIndex == find("Price - Lowest")) {
                            catalogPage.model = Backend.sortBy(catalogPage.model, Enum.Sorting.SortByPriceLowest)
                            settingsDialog.lastUsedListingSorting = Enum.Sorting.SortByPriceLowest
                        }
                        if(currentIndex == find("Price - Highest")) {
                            catalogPage.model = Backend.sortBy(catalogPage.model, Enum.Sorting.SortByPriceHighest)
                            settingsDialog.lastUsedListingSorting = Enum.Sorting.SortByPriceHighest
                        }
                        /*if(currentIndex == find("")) {
                            catalogPage.model = Backend.
                        }*/
                    }
                }
            } // Row
        } // Rectangle            
        StackLayout {
            id: catalogStack
            ////anchors.fill: parent
            width: childrenRect.width; height: parent.height//width: currentItem.childrenRect.width; height: currentItem.contentHeight//currentItem.childrenRect.height + 400
            anchors.horizontalCenter: parent.horizontalCenter// only works when width = childrenRect.width ////viewToggle.horizontalCenter
            anchors.top: topPanel.bottom; anchors.topMargin: 15
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            currentIndex: viewToggle.currentIndex
            property int pagesCount: 1//10////pages.count// Number of page results from search
            property var currentItem: this.itemAt(this.currentIndex)
            property var pages: currentItem.children[0]
            property int currentPageIndex: 0
            property var pageItem: pages.children[0] // can be either a CatalogGrid or CatalogList
            clip: true // because this is clipped, I cannot see all the grid/list items

            NeroshopComponents.CatalogGrid {
                model: (catalogPage.model != null) ? catalogPage.model : this.model
            }    
            
            NeroshopComponents.CatalogList {
                model: (catalogPage.model != null) ? catalogPage.model : this.model
            }
        }        
        // Custom pagination bar
        /*NeroshopComponents.PaginationBar {
            id: pagination
            firstButton.onClicked: { if(!firstButton.disabled) goToPrevPage() }
            secondButton.onClicked: { if(!secondButton.disabled) goToNextPage() }
            numberField.onEditingFinished: { setCurrentPageIndex(numberField.text - 1) }
            currentIndex: catalogStack.pages.currentIndex//catalogStack.currentIndex
            count: catalogStack.pages.count//catalogStack.count
            // For Row ONLY, NOT RowLayout (use Layout.alignment for RowLayout instead)
            anchors.horizontalCenter: parent.horizontalCenter//anchors.horizontalCenter: catalogStack.horizontalCenter
            anchors.top: catalogStack.bottom; anchors.topMargin: 20//anchors.bottom: parent.bottom; anchors.bottomMargin: 20
            ////Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            ////Layout.bottomMargin: 20
        }*/                        
        //}
} // Page
