import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12

import FontAwesome 1.0
import "." as NeroshopComponents

Column {
    id: catalogList
    width: parent.width; height: (boxHeight * catalogListRepeater.count) // The height is the height of all the models in the listview combined
    spacing: 5
    property bool hideProductDetails: true//false // hides product name, price, and star ratings if set to true
    property real boxWidth: this.width
    property real boxHeight: 300//NeroshopComponents.CatalogGrid.boxWidth
    property alias count: catalogListRepeater.count
             
    Repeater {
        id: catalogListRepeater
        model: 10
        delegate: Rectangle {
            id: productBox
            width: catalogList.boxWidth; height: catalogList.boxHeight // The height of each individual model item/ list element
            color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e" : "#a0a0a0"
            border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            border.width: 0
            radius: 5
            //clip: true
        
            Text {
                text: "I'm item " + index + ", model count: " + catalogListRepeater.count//text: //name + ": " + number
                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
            }
        }     
    }
}
