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
    function isValidPageNumber(pageNumber) {
        return Number.isInteger(pageNumber) && pageNumber >= 1 && pageNumber <= getTotalPages()
    }
    function setPage(newPage) {
        if(isValidPageNumber(newPage)) {
            currentSearchParams.set_page(newPage)
            reloadSearch()
        }
    }
    function goToNextPage() {
        setPage(currentSearchParams.get_page()+1)
    }
    function goToPrevPage() {
        setPage(currentSearchParams.get_page()-1)
    }
    function reloadSearch() {
        page = currentSearchParams.page
        totalResults = Backend.getListingsTotalResults(currentSearchParams)
        totalPages = getTotalPages()
        model = Backend.getListings(currentSearchParams)
    }
    function getTotalPages() {
        return Math.ceil(catalogPage.totalResults/currentSearchParams.count);
    }
    function getItemsCount() {
        // ... boxesPerGrid * pageCount
    }    
    //property alias catalogIndex: catalogStack.currentIndex
    property int totalResults: 0
    property int page: 1
    property int totalPages: 0
    property var model: null

    onTotalResultsChanged: { catalogPage.totalPages = catalogPage.getTotalPages() }

    ScrollView {
        id: catalogScrollView//anchors.margins: 20
        anchors.fill: parent////anchors.top: viewToggle.bottom; anchors.topMargin: 15
        width: parent.width; height: parent.height
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn//ScrollBar.AsNeeded
        clip: true // The area in which the contents of the filterBox will be bounded to (set width and height) // If clip is false then the contents will go beyond/outside of the filterBox's bounds
        contentWidth: this.width//childrenRect.width////mainWindow.width//parent.width
        contentHeight: catalogStack.height + 100//// + 200//100 for viewToggle space and 100 for pagination space //mainWindow.height
        //ColumnLayout {
        //    anchors.fill: parent

        /*NeroshopComponents.FilterBox {
            id: productFilterBox
            ////anchors.left: parent.left
            ////anchors.leftMargin: 20
            ////anchors.top: parent.top
            ////anchors.topMargin: 20
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.leftMargin: 20
            Layout.topMargin: 20
        }*/    
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
                text: qsTr("Page %1 of %2 (Results: %3)").arg(page).arg(totalPages).arg(totalResults)
                font.bold: true
                anchors.left: parent.left
                anchors.verticalCenter: viewToggle.verticalCenter//anchors.top: viewToggle.top
                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            }
            NeroshopComponents.ComboBox {
                id: resultsPerPageBox
                anchors.left: pageResultsDisplay.right
                anchors.leftMargin: 10
                anchors.verticalCenter: viewToggle.verticalCenter
                width: 150
                model: ["5", "10", "15", "25", "50"]
                Component.onCompleted: currentIndex = find(currentSearchParams.get_count().toString())
                displayText: "Show: " + currentText
                onActivated: {
                    if(Number(model[currentIndex]) != currentSearchParams.get_count()) {
                        currentSearchParams.set_page(((currentSearchParams.get_page()-1)*currentSearchParams.get_count()/Number(model[currentIndex]))+1)
                        currentSearchParams.set_count(Number(model[currentIndex]))
                        reloadSearch()
                    }
                }
            }
            // ViewToggle
            NeroshopComponents.ViewToggle {
                id: viewToggle
                anchors.horizontalCenter: parent.horizontalCenter
                //anchors.top: parent.top; anchors.topMargin: 20
            }    
            //GroupBox {
            //        title: qsTr("Sort")
            // SortComboBox
            NeroshopComponents.ComboBox {
                id: sortByBox
                anchors.right: parent.right
                anchors.verticalCenter: viewToggle.verticalCenter
                width: 300
                model: ["None", "Latest", "Oldest", "Alphabetical order", "Price - Lowest", "Price - Highest", "Rating - Highest", "Rating Count - Highest"]
                Component.onCompleted: currentIndex = find("None")
                displayText: "Sort: " + currentText
                onActivated: {
                    if(currentIndex == find("None")) {
                        currentSearchParams.set_order_by_column("")
                        currentSearchParams.set_order_by_ascending(true)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Oldest")) {
                        currentSearchParams.set_order_by_column("date")
                        currentSearchParams.set_order_by_ascending(true)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Latest")) {
                        currentSearchParams.set_order_by_column("date")
                        currentSearchParams.set_order_by_ascending(false)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Alphabetical order")) {
                        currentSearchParams.set_order_by_column("products.name")
                        currentSearchParams.set_order_by_ascending(true)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Price - Lowest")) {
                        currentSearchParams.set_order_by_column("price")
                        currentSearchParams.set_order_by_ascending(true)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Price - Highest")) {
                        currentSearchParams.set_order_by_column("price")
                        currentSearchParams.set_order_by_ascending(false)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Rating - Highest")) {
                        currentSearchParams.set_order_by_column("rating")
                        currentSearchParams.set_order_by_ascending(false)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    if(currentIndex == find("Rating Count - Highest")) {
                        currentSearchParams.set_order_by_column("rating_count")
                        currentSearchParams.set_order_by_ascending(false)
                        currentSearchParams.set_page(1)
                        reloadSearch()
                    }
                    /*if(currentIndex == find("")) {
                        catalogPage.model = Backend.
                    }*/
                }
            }
        } // Rectangle            

        // Pagination mode (Infinite Scroll mode replaces the StackLayout with a ScrollView)
        StackLayout {
            id: catalogStack
            ////anchors.fill: parent
            width: currentItem.childrenRect.width; height: currentItem.childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter////viewToggle.horizontalCenter
            anchors.top: topPanel.bottom; anchors.topMargin: 15
            ////anchors.left: (productFilterBox.visible) ? productFilterBox.right : parent.left
            ////anchors.leftMargin: (productFilterBox.visible) ? 10 : 20            
            ////Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            ////Layout.maximumWidth: catalogScrollView.width
            currentIndex: viewToggle.currentIndex
            property var currentItem: this.itemAt(this.currentIndex)
            property var pages: currentItem.children[0]
            property var pageItem: pages.children[0] // can be either a CatalogGrid or CatalogList
            clip: true
            // Infinite Scroll Mode (without pages)
            Item {
                id: gridItem
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                Layout.preferredWidth: childrenRect.width; Layout.preferredHeight: childrenRect.height//width: childrenRect.width; height: childrenRect.height
                StackLayout {
                    id: gridStack
                    currentIndex: 0
                    Repeater {
                        model: currentSearchParams.count
                        delegate: NeroshopComponents.CatalogGrid { // has index property
                            model: (catalogPage.model != null) ? catalogPage.model : this.model
                            //Component.onCompleted: console.log("model",this.model)
                            footer: Item {
                                width: parent.width; height: gridPagination.height////childrenRect.height
                                NeroshopComponents.PaginationBar {
                                    id: gridPagination
                                    firstButton.onClicked: { if(!firstButton.disabled) goToPrevPage() }
                                    secondButton.onClicked: { if(!secondButton.disabled) goToNextPage() }
                                    numberField.onEditingFinished: {
                                        if(isValidPageNumber(Number(numberField.text))) {
                                            setPage((Number(numberField.text)))
                                        } else {
                                            numberField.text = page.toString()
                                        }
                                    }
                                    page: catalogPage.page
                                    totalPages: catalogPage.totalPages
                                    anchors.horizontalCenter: parent.horizontalCenter//anchors.horizontalCenter: catalogStack.horizontalCenter
                                    anchors.top: parent.top; anchors.topMargin: 20//anchors.bottom: parent.bottom; anchors.bottomMargin: 20
                                }
                            }
                        }
                    }
                }
            }
            
            Item {
                id: listItem
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                Layout.preferredWidth: childrenRect.width; Layout.preferredHeight: childrenRect.height//width: childrenRect.width; height: childrenRect.height
                StackLayout {
                    id: listStack
                    currentIndex: 0
                    Repeater {
                        model: currentSearchParams.count
                        delegate: NeroshopComponents.CatalogList {
                            model: (catalogPage.model != null) ? catalogPage.model : this.model
                            //Component.onCompleted: console.log("model",this.model)
                            footer: Item {
                                width: parent.width; height: listPagination.height////childrenRect.height
                                NeroshopComponents.PaginationBar {
                                    id: listPagination
                                    firstButton.onClicked: { if(!firstButton.disabled) goToPrevPage() }
                                    secondButton.onClicked: { if(!secondButton.disabled) goToNextPage() }
                                    numberField.onEditingFinished: {
                                        if(isValidPageNumber(Number(numberField.text))) {
                                            setPage((Number(numberField.text)))
                                        } else {
                                            numberField.text = page.toString()
                                        }
                                    }
                                    page: catalogPage.page
                                    totalPages: catalogPage.totalPages
                                    anchors.horizontalCenter: parent.horizontalCenter//anchors.horizontalCenter: catalogStack.horizontalCenter
                                    anchors.top: parent.top; anchors.topMargin: 20//anchors.bottom: parent.bottom; anchors.bottomMargin: 20
                                }
                            }                        
                        }
                    }
                }
            }
        } // StackLayout
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
    } // ScrollView or Flickable
} // Page
