import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Item {
    id: searchBar
    width: childrenRect.width; height: childrenRect.height
    TextField {
        id: searchField
        color: "#ffffff" // textColor
        width: 400; height: 40
        selectByMouse: true
        placeholderText: qsTr("Search")
        
        background: Rectangle { 
            color: "#050506"
            radius: 5
        }
        onTextChanged: {//https://stackoverflow.com/questions/70284407/detect-changes-on-every-character-typed-in-a-textfield-qml
            //console.log("Show search suggestions popup list")
        }
    }

    Button {
        id: searchButton
        text: qsTr("Search")
        //onClicked: 
        display: AbstractButton.IconOnly//AbstractButton.TextBesideIcon
        hoverEnabled: true
        anchors.left: searchField.right
        anchors.leftMargin: 5//1
        anchors.top: searchField.top
        width: 50; height: searchField.height
        
        icon.source: "qrc:/images/search.png"//neroshopResourceDir + "/search.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: parent.hovered ? "#66578e" : "#8071a8"//"#40404f"
            radius: searchField.background.radius
        }     

        onClicked: { // causes crash if pressed multiple times at a fast pace (this is probably due to the while loop in Backend.getListings())
            ////if(searchField.length < 1) return;
            console.log("Searching for " + searchField.text)
            navBar.uncheckAllButtons()
            pageLoader.setSource("qrc:/qml/pages/CatalogPage.qml", {"model": Backend.getListings()})//, {"model": [""]})
            //console.log("page Loader Item (CatalogPage):", pageLoader.item)
            //console.log("page Loader Item (CatalogPage.catalog):", pageLoader.catalog)//.item)
        }
    }
}
