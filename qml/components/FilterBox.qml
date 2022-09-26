import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Pane {
    width: 250; height: 540
    background: Rectangle { 
        color: (NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"
        radius: 3
    }
    // conditionGroup
    ButtonGroup {
        id: conditionButtonGroup
        buttons: conditionColumn.children
        exclusive: false // more than one button in the group can be checked at any given time
        checkState: conditionParentBox.checkState
        onClicked: {
            console.log("Selected condition:", button.text)
            if(checkState == Qt.Unchecked) {
                console.log("checkState: NO button is checked")
            }
            if(checkState == Qt.Checked) {
                console.log("checkState: All buttons are checked")
            }            
            if(checkState == Qt.PartiallyChecked) {
                console.log("checkState: One or more buttons are checked")
            }
        }
    }
    // priceGroup
    ButtonGroup {
        id: priceButtonGroup
        buttons: priceColumn.children
        exclusive: false // more than one button in the group can be checked at any given time
        checkState: priceParentBox.checkState
        onClicked: {
            console.log("Selected price range:", button.text)
            if(checkState == Qt.Unchecked) {
                console.log("checkState: NO button is checked")
            }
            if(checkState == Qt.Checked) {
                console.log("checkState: All buttons are checked")
            }            
            if(checkState == Qt.PartiallyChecked) {
                console.log("checkState: One or more buttons are checked")
            }
        }
    }
        
    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true // The area in which the contents of the filterBox will be bounded to (set width and height) // If clip is false then the contents will go beyond/outside of the filterBox's bounds
        contentWidth: columnLayout.width
        contentHeight: columnLayout.height
        ColumnLayout {
            id: columnLayout//filterOptions
            width: flickable.width// - 10//- 20
            Frame {
                Layout.fillWidth: true
                background: Rectangle {
                    color: "transparent"
                    border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" :  "#000000"//"#21be2b"
                    radius: 2
                    opacity: 0.1
                }                
                ColumnLayout {
                    id: conditionColumn
                    width: parent.width
                    Label { 
                        text: qsTr("Condition") 
                        font.bold: true
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"                    
                    }
                    CheckBox { 
                        id: conditionParentBox
                        checkState: conditionButtonGroup.checkState
                        Text {
                            text: qsTr("Any")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                        
                    }
                    CheckBox { 
                        ButtonGroup.group: conditionButtonGroup
                        Text {
                            text: qsTr("New")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                        
                    }
                    CheckBox { 
                        ButtonGroup.group: conditionButtonGroup
                        Text {
                            text: qsTr("Used")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                        
                    }
                    CheckBox { 
                        ButtonGroup.group: conditionButtonGroup
                        Text {
                            text: qsTr("Refurbished/Renewed")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                        
                    }
                }
            }
            Frame {
                Layout.fillWidth: true
                background: Rectangle {
                    color: "transparent"
                    border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" :  "#000000"//"#21be2b"
                    radius: 2
                    opacity: 0.1
                }                
                ColumnLayout {
                    id: priceColumn
                    width: parent.width
                    Label { 
                        text: qsTr("Price") 
                        font.bold: true
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    }
                    // todo: maybe use a slider to set the price range instead?
                    CheckBox { 
                        id: priceParentBox
                        checkState: priceButtonGroup.checkState
                        Text {
                            text: qsTr("Any")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                        
                    }
                    CheckBox { 
                        ButtonGroup.group: priceButtonGroup       
                        Text {
                            text: qsTr("$0.00-$1.00")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                }
            }
        }
        // CatalogPage has a scrollbar so there's no longer any need for this
        /*ScrollBar.vertical: ScrollBar {
            //width: 20
            policy: ScrollBar.AsNeeded//ScrollBar.AlwaysOn
        }*/
    }
    // todo: sort by category, price, ratings, brand, color, etc.
}
