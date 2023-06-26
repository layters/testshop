import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12 // ColorOverlay
import Qt.labs.platform 1.1 // FileDialog

import FontAwesome 1.0

//import neroshop.namespace 1.0

import "../../components" as NeroshopComponents

Page {
    id: sellerHub
    background: Rectangle {
        color: "transparent"
    }
        
    NeroshopComponents.TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        model: ["Overview", "Inventory", "Customers"]
        Component.onCompleted: {
            buttonAt(0).checked = true //TODO: make "Inventory" default tab for quicker access to listing products
        }
    }
            
    StackLayout { // TODO: stackview (dashboard) with sections: overview (stats and analytics), inventory management (listing and delisting products), customer order management, reports, etc.
        id: dashboard
        width: parent.width; height: parent.height//2000//anchors.fill: parent
        anchors.top: tabBar.bottom
        anchors.left: parent.left; anchors.right: parent.right // margins can now be applied horizontally
        anchors.margins: 20
        currentIndex: tabBar.currentIndex
            
        Item {
            id: overviewTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
            ScrollView {
                id: overviewTabScrollable
                anchors.fill: parent
                contentWidth: parent.childrenRect.width//parent.width
                contentHeight: parent.childrenRect.height + 100//* 2
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                clip: true
                
                ColumnLayout {                
                    anchors.horizontalCenter: parent.horizontalCenter//
                    spacing: 20
                    RowLayout {
                        id: stats
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop// <- this is not working :/
                        property real numberTextFontSize: 24
                        property string textColor: "#384364"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#384364"
                        property real boxWidth: 250//(scrollView.width / 3) - 20//scrollView.width / statsRepeater.count
                        property real boxHeight: 110
                        property real boxRadius: 6
                        spacing: 15
                        property bool useDefaultBoxColor: true//false
                        property string boxColor: "#ffffff"////(NeroshopComponents.Style.darkTheme) ? "#384364" : "#ffffff"
                        // Products (listed)
                        Rectangle {
                            Layout.preferredWidth: stats.boxWidth
                            Layout.preferredHeight: stats.boxHeight
                            color: stats.useDefaultBoxColor ? stats.boxColor : "#e9eefc"
                            radius: stats.boxRadius
                    
                            Item {
                                anchors.fill: parent
                                anchors.centerIn: parent
                        
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left; anchors.leftMargin: width / 2
                                    width: 64; height: 64
                                    color: "#e9eefc"
                                    radius: 50
                                    Image {
                                        id: productIcon
                                        source: "qrc:/assets/images/open_parcel.png"
                                        width: 32; height: 32
                                        anchors.centerIn: parent
                                    }
                            
                                    ColorOverlay {
                                        anchors.fill: productIcon
                                        source: productIcon
                                        color: "#4169e1"
                                        visible: productIcon.visible
                                    }
                                }
                            
                                Item {
                                    anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                    anchors.top: parent.children[0].top
                            
                                    Text {
                                        text: User.productsCount//"0"//"5430"
                                        font.bold: true
                                        font.pointSize: stats.numberTextFontSize
                                        color: stats.textColor
                                    }
                    
                                    Text {
                                        text: "Products"
                                        //font.bold: true
                                        color: stats.textColor
                                        anchors.left: parent.children[0].left
                                        anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                    }
                                }
                            }
                        }
                        // Sales (the total number of completed orders)
                        Rectangle {
                            Layout.preferredWidth: stats.boxWidth
                            Layout.preferredHeight: stats.boxHeight
                            color: stats.useDefaultBoxColor ? stats.boxColor : "#eff5ef"
                            radius: stats.boxRadius
                    
                            Item {
                                anchors.fill: parent
                                anchors.centerIn: parent
                        
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left; anchors.leftMargin: width / 2
                                    width: 64; height: 64
                                    color: "#eff5ef"
                                    radius: 50
                                    Image {
                                        id: salesIcon
                                        source: "qrc:/assets/images/increase.png"
                                        width: 32; height: 32
                                        anchors.centerIn: parent
                                    }
                            
                                    ColorOverlay {
                                        anchors.fill: salesIcon
                                        source: salesIcon
                                        color: "#8fbc8f"
                                        visible: salesIcon.visible
                                    }
                                }
                        
                                Item {
                                    anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                    anchors.top: parent.children[0].top
                            
                                    Text {
                                        text: "0"//"17440"
                                        font.bold: true
                                        font.pointSize: stats.numberTextFontSize
                                        color: stats.textColor
                                    }
                    
                                    Text {
                                        text: "Sales"
                                        //font.bold: true
                                        color: stats.textColor
                                        anchors.left: parent.children[0].left
                                        anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                    }
                                }
                            }
                        }
                        // Ratings/Feedback/Reputation
                        Rectangle {
                            Layout.preferredWidth: stats.boxWidth
                            Layout.preferredHeight: stats.boxHeight
                            color: stats.useDefaultBoxColor ? stats.boxColor : "#fffbe5"
                            radius: stats.boxRadius
                    
                            Item {
                                anchors.fill: parent
                                anchors.centerIn: parent
                        
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left; anchors.leftMargin: width / 2
                                    width: 64; height: 64
                                    color: "#fffbe5"
                                    radius: 50
                                    Image {
                                        id: ratingIcon
                                        source: "qrc:/assets/images/rating.png"
                                        width: 32; height: 32
                                        anchors.centerIn: parent
                                    }
                            
                                    ColorOverlay {
                                        anchors.fill: ratingIcon
                                        source: ratingIcon
                                        color: "#ffd700"//"#e6c200"
                                        visible: ratingIcon.visible
                                    }
                                }
                        
                                Item {
                                    anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                    anchors.top: parent.children[0].top
                            
                                    Text {
                                        text: qsTr("%1%2").arg(User.getReputation()).arg("%")
                                        font.bold: true
                                        font.pointSize: stats.numberTextFontSize
                                        color: stats.textColor
                                    }
                    
                                    Text {
                                        text: "Reputation"
                                        //font.bold: true
                                        color: stats.textColor
                                        anchors.left: parent.children[0].left
                                        anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                    }
                                }
                            }
                        }
                    } // Row
                // TODO: show recent/pending orders from customers, show seller's top-selling products, show customer reviews on seller products
                } // ColumnLayout
            } // ScrollView 
        } // overview tab
            
        Item {
            id: inventoryTab
            // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true

            ScrollView {
                id: inventoryTabScrollable
                anchors.fill: parent
                contentWidth: width
                contentHeight: inventoryTab.childrenRect.height * 3//+ tabBar.buttonHeight + 20//parent.height
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                clip: true
                
                ColumnLayout {
                    id: inventoryManager
                    width: parent.width////; height: childrenRect.height////anchors.fill: parent
                    ////spacing: 5
                    // Add product button
                    Item {
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                                                
                        Row {
                            width: childrenRect.width; height: maxHeight
                            layoutDirection: Qt.RightToLeft
                            spacing: 20 // Spacing between each Row item
                            property real maxHeight: 100//200
                                    
                            Button {
                                id: addProductButton
                                ////anchors.right: parent.right // <- uncomment if not enclosed in row
                                text: qsTr("+ Add Product")
                                width: 304/*inventoryManager.width*/; height: 100//width: 200; height: 100//width: 100; height: width
                                hoverEnabled: true
                                background: Rectangle {
                                    radius: 5
                                    color: parent.hovered ? (parent.down ? "#5c7b1e" : "#759b26") : "#698b22"
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#ffffff"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: productDialog.open()
                            } // TODO: Import/Export button maybe?
                        }
                    }
                    // Column 2
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Layout.alignment: Qt.AlignTop
                        Component.onCompleted: console.log("Column 2 biggest height:", childrenRect.height) // no children should exceed the biggest height. All children height must match at least
                        Row { // Row 1 (left)
                            anchors.left: parent.left
                            spacing: 15
                            // Filter box - hard to implement so disable for now
                            /*NeroshopComponents.ComboBox {
                                width: 200
                                currentIndex: 0
                                displayText: (currentIndex == find("None")) ? "Filter by " : currentText
                                model: ["None", "Date"]
                                onActivated: {
                                    if(currentIndex == find("None")) {
                                        inventoryTable.list.model = User.inventory
                                    }
                                    if(currentIndex == find("Date")) {
                                        inventoryTable.list.model = User.inventoryDate//User.getInventory(Neroshop.SortByDateOldest)
                                    }
                                }
                            }*/
                            // TODO: Checkbox with "show only in stock items" option
                            // Global actions (applies to selected items)
                            Frame {
                                height: 40
                                background: Rectangle {
                                    color: "transparent"
                                    border.color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#181a1b") : "#bdbdbd"////"#c9c9cd" // default frame border color is "#bdbdbd"
                                    radius: 2
                                }
                                Row {
                                    anchors.fill: parent
                                    spacing: 5
                                    Button {
                                        anchors.verticalCenter: parent.verticalCenter
                                        //width: 32; height: 32
                                        text: qsTr("Remove selection")
                                        display: AbstractButton.IconOnly
                                        hoverEnabled: true
        
                                        icon.source: "qrc:/assets/images/trash.png"
                                        icon.color: "#b22222"
                                        background: Rectangle {
                                            color: parent.hovered ? ((NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#181a1b") : "#c9c9cd") : "transparent"
                                            radius: 5
                                        }
                                        NeroshopComponents.Hint {
                                            visible: parent.hovered
                                            height: contentHeight + 20; width: contentWidth + 20
                                            text: parent.text
                                            pointer.visible: false
                                            timeout: 1000; delay: 0
                                        }
                                        MouseArea { 
                                            anchors.fill: parent
                                            onPressed: mouse.accepted = false // without this, Button.onClicked won't work
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                        onClicked: {
                                            if(inventoryTable.getSelectionCount() <= 0) return;
                                            removeProductsMessageBox.open()
                                        }
                                    }
                                }
                            }
                        }
                        Row { // Row 2 (right)
                            anchors.right: parent.right
                            layoutDirection: Qt.RightToLeft
                            /*// Search bar - not usable ATM
                            TextField {
                                id: inventorySearchField
                                color: "#0e0e11"//"#ffffff"//"dimgray" // textColor
                                anchors.verticalCenter: parent.verticalCenter
                                width: 200; height: 40
                                selectByMouse: true
                                placeholderText: qsTr("Search inventory"); placeholderTextColor: "#989999"
        
                                background: Rectangle { 
                                    color: "#eeeef1"
                                    radius: 5
                                }
                                rightPadding: 25 + searchIcon.width
                                Text {
                                    id: searchIcon
                                    text: qsTr("\uf002")
                                    color: hovered ? "#7c6da6" : "#989999"
                                    font.bold: true
                                    font.family: FontAwesome.fontFamily
                                    anchors.right: parent.right
                                    anchors.rightMargin: 15
                                    anchors.verticalCenter: parent.verticalCenter
                                    property bool hovered: false
                                    MouseArea { 
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: console.log("Searching for " + inventorySearchField.text);
                                        onEntered: searchIcon.hovered = true;
                                        onExited: searchIcon.hovered = false;
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                                Keys.onEnterPressed: console.log("Enter pressed")
                                Keys.onReturnPressed: console.log("Return pressed")
                            }*/
                        }
                    }        
                    // Column 3
                    NeroshopComponents.CheckBox {
                        id: showOutOfStockProductsBox
                        Layout.preferredWidth: 20; Layout.preferredHeight: 20//anchors.verticalCenter: parent.verticalCenter
                        shapeType: NeroshopComponents.CheckBox.ShapeType.Block
                        text: qsTr("Show out of stock products")
                        textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        color: "transparent"
                        checked: true
                    }                                
                    // Inventory table                    
                    NeroshopComponents.InventoryTable {
                        id: inventoryTable
                        Layout.fillWidth: true
                    }
                    // ProductDialog popup
                    NeroshopComponents.ProductDialog {
                        id: productDialog
                        anchors.centerIn: Overlay.overlay////parent
                    }
                    // RemoveProduct message prompt
                    NeroshopComponents.MessageBox {
                        id: removeProductsMessageBox
                        x: mainWindow.x + (mainWindow.width - this.width) / 2
                        y: mainWindow.y + (mainWindow.height - this.height) / 2
                        title: qsTr("Remove product")
                        text: qsTr("Are you sure you want to permanently remove the selected product(s)?")
                        buttonModel: ["No", "Yes"]
                        buttonRow.state: "centered"; buttonRow.width: 300 // buttons should fill the row width
                        Component.onCompleted: {
                            buttonAt(0).onClickedCallback = function() { close() }
                            buttonAt(1).color = "#4169e1"//"#4682b4"
                            buttonAt(1).onClickedCallback = function() {
                                inventoryTable.removeSelectedItems()
                                close()
                            }
                        }    
                    }
                }
            } // ScrollView
        } // inventoryTab
            
        Item {
            id: customerOrdersTab
            //width: parent.width; height: childrenRect.height////anchors.fill: parent
            // TODO: order status: pending, unpaid, delivered/completed, refunded, etc.
            /*ScrollView {
                id: customerOrdersTabScrollable
                anchors.fill: parent//anchors.margins: 20
                contentWidth: parent.childrenRect.width//parent.width
                contentHeight: parent.childrenRect.height * 3//parent.height
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn////AsNeeded
                clip: true*/
                
                /*ColumnLayout {
                    id: orders
                    width: parent.width//; height: childrenRect.height
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.margins: 20//anchors.leftMargin: 10; anchors.rightMargin: 10
                    
                    NeroshopComponents.Table {
                        id: orderRequestTable
                        Layout.preferredWidth: parent.width////Layout.fillWidth: true
                        Layout.preferredHeight: 500////Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }*/
            ////}            
        } 
    } // StackLayout
}
