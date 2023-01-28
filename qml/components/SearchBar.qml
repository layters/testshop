import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Item {
    TextField {
        id: searchBar
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
        anchors.left: searchBar.right
        anchors.leftMargin: 5//1
        anchors.top: searchBar.top
        width: 50; height: searchBar.height
        
        icon.source: "qrc:/images/search.png"//neroshopResourceDir + "/search.png"
        icon.color: "#ffffff"
                        
        background: Rectangle {
            color: parent.hovered ? "#66578e" : "#8071a8"//"#40404f"
            radius: searchBar.background.radius
        }     

        onClicked: {
            console.log("Searching for " + searchBar.text)
            pageLoader.setSource("qrc:/qml/pages/CatalogPage.qml")//, {"catalogIndex": 0})//, {"model": [""]})
            //console.log("page Loader Item (CatalogPage):", pageLoader.item)
            //console.log("page Loader Item (CatalogPage.catalog):", pageLoader.catalog)//.item)
        }
    }
}
