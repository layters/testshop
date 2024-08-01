import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import FontAwesome 1.0

import neroshop.Enums 1.0

import "." as NeroshopComponents

Item {
    id: table
    property real titleBoxRadius: 3
    property real titleBoxSpacing: titleBar.spacing
    property string columnBorderColor: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"
    property string columnColor: listView.cellColor//"transparent"//(NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#0e0e11" : "#101010") : "#f0f0f0"////"transparent"//"#6c6c6f"
    property alias list: listView
    function removeSelectedItems() { listView.removeSelectedItems() }
    function getSelectionCount() { const selection_count = listView.getSelectedItems().length; return selection_count }
    //width: childrenRect.width; height: childrenRect.height
    ColumnLayout {
        id: tableLayout
        anchors.fill: parent
        spacing: 2
        
        RowLayout {
            id: titleBar
            Layout.fillWidth: true
            //spacing: 2
            ButtonGroup {
                id: childGroup
                exclusive: false
                checkState: parentBox.checkState // https://doc.qt.io/qt-6/qml-qtquick-controls2-checkbox.html#checkState-prop
            }
    
            Rectangle {
                id: checkBoxColumn
                Layout.preferredWidth: parentBox.width + 50
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                NeroshopComponents.CheckBox {
                    id: parentBox
                    anchors.centerIn: parent
                    checkState: childGroup.checkState
                    contentItem: null
                    color: NeroshopComponents.Style.getColorsFromTheme()[0]
                }   
            }            
            Rectangle {
                id: productImageColumn
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                Label {
                    text: qsTr("Item")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                id: productNameColumn
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                Label {
                    text: qsTr("Name")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                id: priceColumn
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                Label {
                    text: qsTr("Price")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                id: productStockQtyColumn
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                Label {
                    text: qsTr("Quantity")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Rectangle {
                id: actionsColumn
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                Label {
                    text: qsTr("Actions")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }    
            /*Rectangle {
                id: ?Column
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                color: table.columnColor
                border.color: table.columnBorderColor
                radius: table.titleBoxRadius
                Label {
                    text: qsTr("?")
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }*/                                                          
        }
        
        ListView {
            id: listView
            //clip: true
            Layout.fillWidth: true//Layout.preferredWidth: 800; 
            Layout.preferredHeight: listView.cellHeight * listView.count///Layout.fillHeight: true
            property real cellHeight: 100
            property real cellRadius: 0////table.titleBoxRadius
            property string cellColor: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#181a1b") : "#c9c9cd"//"transparent"//table.columnColor
            //property int currentSorting: 0
            ////contentHeight: childrenRect.height
            ////Component.onCompleted: console.log(Neroshop.InventorySorting) // C++ enum
            ScrollBar.vertical: ScrollBar { } // ?
            function getSelectedItems() {
                let selectedItems = []
                for(let i = 0; i < listView.count; i++) {
                    let listItem = listView.itemAtIndex(i) // itemAtIndex is only available in Qt 5.13+ :(
                    let isItemChecked = (listItem == null) ? false : listItem.children[1].children[0].checked
                    if(isItemChecked) {
                        selectedItems[i] = listView.itemAtIndex(i)
                        //console.log(selectedItems[i].children[1].children[2].text, " selected")
                    }
                }
                return selectedItems
            }
            function getSelectedItemsListingKeys() {
                let selectedItemsListingKeys = []
                for(let index = 0; index < listView.count; ++index) {
                    let listItem = listView.itemAtIndex(index)
                    if(listItem == null || listItem == undefined) continue; // skip invalid values
                    if(listItem.checked) {
                        selectedItemsListingKeys[index] = listItem.listingKey
                        console.log((listItem.children[1].children[2].text) + (" (" + listItem.listingKey + ")"), " selected")
                    }
                }
                return selectedItemsListingKeys
            }
            function removeSelectedItems() {
                const selectedItemsListingKeys = getSelectedItemsListingKeys()
                User.delistProducts(selectedItemsListingKeys)
                // User.delistProduct fails to delete all selected items and only deletes one of the selected item (at the top of list)
                // This is due to the model changing every time we emit the signal
                // Hence the creation of User.delistProducts (notice the s) which solves the issue
            }
            model: showOutOfStockProductsBox.checked ? User.inventory : User.getInventory(Enum.Sorting.SortByAvailability)
            delegate: Rectangle {
                width: listView.width
                height: listView.cellHeight
                color: listView.cellColor
                border.color: "transparent"//table.columnBorderColor
                radius: listView.cellRadius
                property string listingKey: modelData.key
                property alias rowItem: delegateRow
                property bool checked: delegateRow.children[0].checked
                Rectangle {
                    anchors.fill: parent
                    color: delegateRow.children[0].checked ? "#8071a8" : "transparent"
                    radius: parent.radius
                }
                
                Item {////Row {
                    id: delegateRow
                    anchors.fill: parent
                    NeroshopComponents.CheckBox {
                        x: checkBoxColumn.x + (checkBoxColumn.width - this.width) / 2//checkBoxColumn.x
                        anchors.verticalCenter: parent.verticalCenter
                        ButtonGroup.group: childGroup
                        width: parentBox.width; height: width
                        color: parentBox.color
                    }
                    Rectangle {
                        id: productImageBox
                        x: productImageColumn.x
                        width: productImageColumn.width; height: parent.height
                        color: "transparent"
                        //border.color: "royalblue"
                        Image {
                            id: productImage
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width; height: parent.height - 10
                            fillMode: Image.PreserveAspectFit
                            mipmap: true
                            asynchronous: true
                            onStatusChanged: {
                                if (productImage.status === Image.Error) {
                                    // Handle the error by displaying a fallback or placeholder image
                                    source = "image://listing?id=%1&image_id=%2".arg(modelData.key).arg("thumbnail.jpg")
                                }
                            }
                            // Wait for a short delay before attempting to load the actual image (which is created after its source is loaded :X)
                            Timer {
                                id: imageTimer
                                interval: 200
                                running: true
                                repeat: false
                                onTriggered: {
                                    productImage.source = "image://listing?id=%1&image_id=%2".arg(modelData.key).arg(modelData.product_images[0].name)
                                }
                            }
                        }
                    }     
                    Label {
                        x: productNameColumn.x + 5////productNameColumn.x + (productNameColumn.width - this.width) / 2//productNameColumn.x
                        width: productNameColumn.width
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr(modelData.product_name)//.arg(index)
                        ////anchors.leftMargin: (productNameColumn.x + (productNameColumn.width - this.width) / 2)// + titleBar.spacing////Layout.fillWidth: true
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                        elide: Label.ElideRight
                    }    
                    Label {
                        x: priceColumn.x + (priceColumn.width - this.width) / 2
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("%1%2 %3").arg(Backend.getCurrencySign(modelData.currency)).arg(Number(modelData.price).toFixed(Backend.getCurrencyDecimals(modelData.currency))).arg(modelData.currency)//modelData.price
                        ////Layout.fillWidth: true
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                        elide: Label.ElideRight
                    }
                    Item {
                        x: productStockQtyColumn.x + (productStockQtyColumn.width - this.width) / 2//productStockQtyColumn.x
                        anchors.verticalCenter: parent.verticalCenter
                        width: children[0].width; height: children[0].height
                        TextField {
                            id: quantityField
                            width: contentWidth + 20
                            text: modelData.quantity
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            font.bold: true
                            selectByMouse: editIcon.activated ? true : false
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: RegExpValidator{ regExp: /[0-9]*/ }
                            readOnly: editIcon.activated ? false : true
                            maximumLength: 9
                            property string originalText: qsTr("")
                            background: Rectangle { 
                                color: "transparent"
                                border.color: !parent.readOnly ? ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000") : "transparent"
                                border.width: parent.activeFocus ? 2 : 1
                                radius: 3
                            }
                            function adjustQuantity() {
                                if(Number(text) >= 999999999) {
                                    text = 999999999
                                }
                                if(Number(this.text) <= 1) {
                                    text = 1
                                }
                            }
                            Component.onCompleted: {
                                quantityField.originalText = text
                            }
                            /*onEditingFinished: { // Pretty much acts like onFocusChanged
                                if(editIcon.activated == true) {
                                    adjustQuantity()
                                    User.setStockQuantity(modelData.key, text);
                                    editIcon.activated = false
                                    console.log("Finished editing")
                                }
                            }
                            onFocusChanged: {
                                if(!focus) {}
                            }*/
                        }
                        Button {
                            id: saveQuantityButton
                            anchors.right: parent.right
                            anchors.rightMargin: -(width + 5)
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Save")
                            width: contentItem.contentWidth + 20; height: 32
                            hoverEnabled: true
                            visible: !quantityField.readOnly
                            background: Rectangle {
                                color: parent.hovered ? "#698b22" : "#506a1a"
                                radius: 5
                            }
                            contentItem: Text {
                                text: parent.text//qsTr(FontAwesome.check)
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                quantityField.adjustQuantity()
                                User.setStockQuantity(modelData.key, quantityField.text);
                                editIcon.activated = false
                                console.log("quantityField.originalText",quantityField.originalText)
                                console.log("Saved")
                            }
                            MouseArea { 
                                anchors.fill: parent
                                onPressed: mouse.accepted = false // without this, Button.onClicked won't work
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    } // Item
                    Text {
                        id: removeButton // TODO: rename to removeIcon?
                        x: expirationIcon.visible ? (actionsColumn.x + (actionsColumn.width - (this.width + editIcon.width + 10 + expirationIcon.width + 10)) / 2) : (actionsColumn.x + (actionsColumn.width - (this.width + editIcon.width + 10)) / 2)
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr(FontAwesome.trashCan)
                        color: "#b22222"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        font.pointSize: 16
                        property bool hovered: false
                        NeroshopComponents.Hint {
                            visible: parent.hovered
                            height: contentHeight + 20; width: contentWidth + 20
                            text: qsTr("Remove")
                            pointer.visible: false
                            timeout: 1000; delay: 0
                        }
                        MouseArea { 
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.hovered = true
                            onExited: parent.hovered = false
                            onClicked: User.delistProduct(modelData.key)
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                    // Pencil
                    Text {
                        id: editIcon
                        x: removeButton.x + removeButton.width + 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr(FontAwesome.pencil)
                        color: !editIcon.activated ? "#808080" : "#ffc60d"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        font.pointSize: 16
                        property bool hovered: false
                        //visible: !modelData.hasOwnProperty("expiration_date")
                        property bool activated: false
                        NeroshopComponents.Hint {
                            visible: editIcon.hovered
                            height: contentHeight + 20; width: contentWidth + 20
                            text: editIcon.activated ? qsTr("Stop editing") : qsTr("Edit")
                            pointer.visible: false
                            timeout: 1000; delay: 0
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.hovered = true
                            onExited: parent.hovered = false
                            onClicked: {
                                parent.activated = !parent.activated
                                if(parent.activated == false) {
                                    quantityField.text = quantityField.originalText // Restore original text (not saved)
                                }
                            }
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                    // Clock
                    Text {
                        id: expirationIcon
                        x: /*!editIcon.visible ? (removeButton.x + removeButton.width + 10) : */(editIcon.x + editIcon.width + 10)
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr(FontAwesome.clock)
                        color: "royalblue"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                        font.pointSize: 16
                        property bool hovered: false
                        visible: modelData.hasOwnProperty("expiration_date")
                        property string message: {
                            if(modelData.hasOwnProperty("expiration_date")) { 
                                if(Backend.getDurationFromNow(modelData.expiration_date) != "") {
                                    return "Expires in %1".arg(Backend.getDurationFromNow(modelData.expiration_date))
                                }
                            }
                            return "Expired"
                        }
                        NeroshopComponents.Hint {
                            visible: expirationIcon.hovered
                            height: contentHeight + 20; width: contentWidth + 20
                            text: expirationIcon.message
                            pointer.visible: false
                            timeout: 3000; delay: 0
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.hovered = true
                            onExited: parent.hovered = false
                        }
                    }
                }                
            }
        }
    } 
}
