import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12

import neroshop.Enums 1.0

import "." as NeroshopComponents

Item {
    id: searchBar
    width: childrenRect.width; height: childrenRect.height
    property var model: Backend.getListingsBySearchTerm(searchField.text, settingsDialog.hideIllegalProducts)
    TextField {
        id: searchField
        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"// textColor
        width: 400; height: 40
        selectByMouse: true
        placeholderText: qsTr("Search")
        
        background: Rectangle { 
            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#efefef"
            radius: 5
        }
        Keys.onEnterPressed: searchButton.activate()
        Keys.onReturnPressed: searchButton.activate()
    }

    Button {
        id: searchButton
        text: qsTr("Search")
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        hoverEnabled: true
        anchors.left: searchField.right
        anchors.leftMargin: 5//1
        anchors.top: searchField.top
        width: 50; height: searchField.height
        property alias cursorShape: mouseArea.cursorShape
        
        icon.source: "qrc:/assets/images/search.png"//neroshopResourceDir + "/search.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: parent.hovered ? "#66578e" : "#8071a8"//"#40404f"
            radius: searchField.background.radius
        }     
        
        function activate() { 
            ////if(searchField.length < 1) return;
            searchButton.forceActiveFocus()
            console.log("Searching for " + searchField.text)
            navBar.uncheckAllButtons()
            suggestionsPopup.close()
            pageStack.pushPageWithProperties("qrc:/qml/pages/CatalogPage.qml", {"model": (searchField.text.length < 1) ? Backend.getListings(Enum.Sorting.SortNone, settingsDialog.hideIllegalProducts) : searchBar.model }, StackView.Immediate)//, {"model": [""]})
            //console.log("page Loader Item (CatalogPage):", pageLoader.item)
            //console.log("page Loader Item (CatalogPage.catalog):", pageLoader.catalog)//.item)
        
        }
        
        onClicked: {
            searchButton.activate()
        }
        
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onPressed: mouse.accepted = false
            cursorShape: Qt.PointingHandCursor
        }
    }
    
    Popup {
        id: suggestionsPopup
        width: searchField.width// + (searchButton.anchors.leftMargin + searchButton.width)
        height: Math.min(suggestionsList.contentHeight, (suggestionsList.delegateHeight * suggestionsPopup.maxSuggestions))
        y: searchField.height + 1
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        visible: (searchField.activeFocus && searchField.text.length > 0 && suggestionsList.count > 0) ? true : false
        background: Rectangle {
            color: suggestionsList.interactive ? "transparent" : "red"
        }
        property int maxSuggestions: 10 // This is the max suggestions shown on the scrollview at a time rather

        ListView {
            id: suggestionsList
            anchors.centerIn: parent
            width: suggestionsPopup.width
            height: suggestionsPopup.height
            clip: true
            interactive: false // Set to true for scrollbar to work with mouse wheel (but then it flicks x.x)
            ScrollBar.vertical: ScrollBar { }
            property real delegateHeight: 32
            model: {
                let uniqueProductNames = [];

                for (let i = 0; i < searchBar.model.length; i++) {
                    let item = searchBar.model[i];
                    let lowercaseName = item.product_name.toLowerCase(); // Convert to lowercase
                    if (uniqueProductNames.indexOf(lowercaseName) === -1) {
                        uniqueProductNames.push(lowercaseName);
                    }
                }

                return uniqueProductNames;
            }//.slice(0, suggestionsPopup.maxSuggestions) // Limit to the first 10 items
            delegate: Rectangle {
                width: suggestionsList.width
                height: suggestionsList.delegateHeight
                color: hovered ? NeroshopComponents.Style.getColorsFromTheme()[1] : NeroshopComponents.Style.getColorsFromTheme()[0]
                property bool hovered: false
                
                Text {
                    text: modelData
                    anchors.fill: parent
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: "#ffffff"
                }
                
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        hovered = true
                    }
                    onExited: {
                        hovered = false
                    }
                    onClicked: {
                        searchField.text = modelData
                        searchButton.activate() // This does not work either :( ////suggestionsPopup.close() // Does not work :( => "ReferenceError: suggestionsPopup is not defined"
                    }
                }
            }
        }
    } // Popup
}
