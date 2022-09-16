import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "." as NeroshopComponents

Pane {//Item {
    background: Rectangle { 
        color: "#a0a0a0"//(NeroshopComponents.Style.darkTheme) ? "#2e2e2e"/*"#121212"*/ : "#a0a0a0"
    }    
    
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

    ScrollView { // inherits Pane
        //ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        //ScrollBar.vertical.policy: ScrollBar.AlwaysOn    
        width: parent.width + 50; height: 500
    
        Column {//ColumnLayout {
            id: conditionColumn
            anchors.fill: parent
            Text { 
                text: qsTr("Condition"); 
                font.bold: true 
            }
            CheckBox {
                id: conditionParentBox
                text: qsTr("Any")//qsTr("Parent")
                checkState: conditionButtonGroup.checkState
            }        
            CheckBox { 
                //checked: false
                text: qsTr("New")
                leftPadding: indicator.width
                ButtonGroup.group: conditionButtonGroup
            }
            CheckBox { 
                text: qsTr("Used")
                leftPadding: indicator.width
                ButtonGroup.group: conditionButtonGroup
            }
            CheckBox { 
                text: qsTr("Refurbished/Renewed")
                leftPadding: indicator.width
                ButtonGroup.group: conditionButtonGroup
            }
        }
        // todo: sort by category, price, ratings, brand, color, etc.
    //////////////
    /*    ColumnLayout {
            id: aColumn
            //anchors.fill: parent
            Text { 
                qsTr("a")
                font.bold: true 
            }
            CheckBox { 
                id: aParentBox
                text: qsTr("Any")
                checkState: aButtonGroup.checkState
            }
            //CheckBox { 
            //    text: qsTr("")
            //    leftPadding: indicator.width
            //    ButtonGroup.group: aButtonGroup            
            //}
        }*/
    }
    /*ButtonGroup {
        id: aButtonGroup
        buttons: aColumn.children
        exclusive: true
        onClicked: {
            console.log("Selected ", button.text)
        }
    }
    ScrollView {
    ColumnLayout {
        id: aColumn
        //anchors.fill: parent
        Text { font.bold: true }
        CheckBox { text: qsTr("Test") }
        //CheckBox { text: qsTr("") }
    }
    }*/
}
