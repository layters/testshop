import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

import "." as NeroshopComponents

Pane {
    width: 250; height: (parent.height - 100)//1000
    background: Rectangle { 
        color: "transparent"////(NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"
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
    // ratingsGroup
    ButtonGroup {
        id: ratingsButtonGroup
        //buttons: ratingsColumn.children
        exclusive: false // more than one button in the group can be checked at any given time
        checkState: ratingsParentBox.checkState
        onClicked: {
            console.log("Selected ratings option:", button.text)
        }
    }  
    /*// brandGroup
    ButtonGroup {
        id: ?ButtonGroup
        //buttons: ?Column.children
        exclusive: false // more than one button in the group can be checked at any given time
        checkState: ?ParentBox.checkState
        onClicked: {
            console.log("Selected ?:", button.text)
        }
    }*/  
    // colorGroup
    ButtonGroup {
        id: colorButtonGroup
        //buttons: ?Column.children
        exclusive: false // more than one button in the group can be checked at any given time
        checkState: colorParentBox.checkState
        onClicked: {
            console.log("Selected color:", button.text)
        }
    }       
    // sizeGroup
    /*ButtonGroup {
        id: ?ButtonGroup
        buttons: ?Column.children
        exclusive: false // more than one button in the group can be checked at any given time
        checkState: ?ParentBox.checkState
        onClicked: {
            console.log("Selected ?:", button.text)
        }
    }*/           
        
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
                            text: qsTr("%1 1.00 and under").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                            text: qsTr("%1 5.00 to %1 10.00").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                            text: qsTr("%1 15.00 to %1 20.00").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                            text: qsTr("%1 25.00 to %1 30.00").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                            text: qsTr("%1 35.00 to %1 40.00").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                            text: qsTr("%1 45.00 to %1 50.00").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                            text: qsTr("%1 55.00 and over").arg(Backend.getCurrencySign(priceDisplayText.currency))
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
                    id: ratingsColumn
                    width: parent.width
                    Label { 
                        text: qsTr("Ratings") 
                        font.bold: true
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    }
                    // todo: maybe use a slider to set the . range instead?
                    CheckBox { 
                        id: ratingsParentBox
                        checkState: ratingsButtonGroup.checkState
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
                        ButtonGroup.group: ratingsButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.star)//qsTr("1 stars")
                            color: "#ffb344"
                            font.bold: true
                            font.family: FontAwesome.fontFamily
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: ratingsButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.star + FontAwesome.star)//qsTr("2 stars")
                            color: "#ffb344"
                            font.bold: true
                            font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: ratingsButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.star + FontAwesome.star + FontAwesome.star)//qsTr("3 stars")
                            color: "#ffb344"
                            font.bold: true
                            font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: ratingsButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.star + FontAwesome.star + FontAwesome.star + FontAwesome.star)//qsTr("4 stars")
                            color: "#ffb344"
                            font.bold: true
                            font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: ratingsButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.star + FontAwesome.star + FontAwesome.star + FontAwesome.star + FontAwesome.star)//qsTr("5 stars")
                            color: "#ffb344"
                            font.bold: true
                            font.family: FontAwesome.fontFamily                            
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
                    id: colorColumn
                    width: parent.width
                    Label { 
                        text: qsTr("Color") 
                        font.bold: true
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    }
                    // todo: maybe use a slider to set the . range instead?
                    CheckBox { 
                        id: colorParentBox
                        checkState: colorButtonGroup.checkState
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
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#ff0000"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#ffa500"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#ffff00"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#008000"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#1560bd"//"#0000ff"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#4b0082"//"#800080"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#ffc0cb"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#664c28"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#808080"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#000000"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
                            anchors.left: parent.right
                            anchors.leftMargin: 0
                            anchors.top: parent.top
                            anchors.topMargin: (parent.height - this.contentHeight) / 2
                        }                    
                    }
                    CheckBox { 
                        ButtonGroup.group: colorButtonGroup       
                        Text {
                            text: qsTr(FontAwesome.square)
                            color: "#ffffff"
                            font.bold: true
                            //font.family: FontAwesome.fontFamily                            
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
    // todo: sort by category, brand, price, ratings, color, material, size, popularity, etc.
}
