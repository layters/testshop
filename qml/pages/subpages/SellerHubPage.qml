import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12 // ColorOverlay

import "../../components" as NeroshopComponents
// TODO: rename this file to Dashboard.qml?
Page {
    id: sellerHub
    background: Rectangle {
        color: "transparent"
    }
    
    NeroshopComponents.TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.topMargin: tabBar.buttonHeight
        anchors.horizontalCenter: parent.horizontalCenter
        model: ["Overview", "Inventory", "Customers"]
        Component.onCompleted: {
            buttonAt(1).checked = true //TODO: make "Inventory" default tab for quicker access to listing products
        }
    }
        
    ScrollView {
        id: scrollView
        width: parent.width; height: 2000//anchors.fill: parent
        anchors.top: tabBar.bottom; anchors.topMargin: tabBar.buttonHeight + 10
        //anchors.margins: 20
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        clip: true
            
        StackLayout { // TODO: stackview (dashboard) with sections: overview (stats and analytics), inventory management (listing and delisting products), customer order management, reports, etc.
            id: dashboard
            anchors.fill: parent
            currentIndex: tabBar.checkedButtonIndex
            
            Item {
                id: overviewTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                
                RowLayout {
                    id: stats
                    anchors.horizontalCenter: parent.horizontalCenter//Layout.alignment: Qt.AlignHCenter | Qt.AlignTop// <- this is not working :/
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
                                    source: "qrc:/images/open_parcel.png"
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
                                    text: "0"//"5430"
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
                                    source: "qrc:/images/increase.png"
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
                                    source: "qrc:/images/rating.png"
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
                                    text: qsTr("%1%2").arg("0").arg("%")
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
                } 
                // TODO: show recent orders from customers, show seller's top-selling products, show customer reviews on seller products
                //ColumnLayout {}
            } // overview tab
            
            Item {
                id: inventoryTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
                Component.onCompleted: { console.log("InventoryTab width",this.width) }
                
                ColumnLayout {//GridLayout {//
                    id: productDetails
                    width: parent.width; height: childrenRect.height////anchors.fill: parent
                    Component.onCompleted: { console.log("ProductDetails width",this.width) }
                    spacing: 30////rowSpacing: 5
                    property string titleTextColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    property real radius: 3
                    property real marginFromTitle: 7
                    property string textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    property string baseColor: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                    property string borderColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#000000"
                    // RegisterItem to "products" table
                    //Product title (TODO: place both text and textfield in an item)
                    // If item fields are related then place them side-by-side in separate columns
                    Item {
                        //Layout.row: 0
                        Layout.alignment: Qt.AlignHCenter// | Qt.AlignTop
                        //Layout.topMargin: 0
                        Layout.preferredWidth: childrenRect.width////productNameField.width
                        Layout.preferredHeight: childrenRect.height // 72 (child margins included)
                        Text {
                            text: "Product name" // height=17
                            color: productDetails.titleTextColor
                            font.bold: true
                        }
                        TextField {
                            id: productNameField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Product title")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }
                        }
                    }
                    // Product price (sales price)
                    Item {
                        //Layout.row: 1
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: "Price"
                            color: productDetails.titleTextColor
                            font.bold: true
                        }                        
                        TextField {
                            id: productPriceField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Enter price")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }                            
                        }
                    }
                    // Product quantity
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: "Quantity"
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        TextField {
                            id: productQuantityField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Enter quantity")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }                            
                        }
                    }                    
                    // Product condition
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: "Condition"
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        ComboBox {
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle   
                            width: 500                     
                            model: ["new", "used", "renewed (refurbished)"]
                        }
                    }                    
                    // Product code UPC, EAN, JAN, SKU, ISBN (for books) // https://www.simplybarcodes.com/barcode_types.html
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: "Product code"
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        TextField {
                            id: productCodeField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Enter product code (optional)")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }                            
                        }
                    }                    
                    // Product categories
                    // Subcategories (will be determined based on selected categories)
                    // Weight
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: ""
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        TextField {
                            id: productWeightField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Enter weight (e.g. 12 lbs.)")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }                            
                        }
                        //ComboBox {
                        //    id: weightMeasurementUnit (default is kg)
                        //}
                    }                    
                    // Size
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: ""
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        TextField {
                            id: productSizeField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Enter size")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }                            
                        }
                        //ComboBox
                    }                    
                    // Variations (i.e. Color, Size options to choose from - optional)
                    // Product location (ship to and ship from)
                    //Product description and bullet points
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: "Description"
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        TextArea {
                            id: productDescArea
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle                            
                            width: 500; height: 250
                        }           
                    }         
                    //Product images
                    //Search terms and relevant keywords (tags)
                    // tags must be separated with a colon
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: ""
                            color: productDetails.titleTextColor
                            font.bold: true
                            //visible: false
                        }
                        TextField {
                            id: productKeywordsField
                            width: 500; height: 50
                            anchors.top: parent.children[0].bottom
                            anchors.topMargin: productDetails.marginFromTitle
                            placeholderText: qsTr("Enter keywords or search term")
                            color: productDetails.textColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: productDetails.baseColor
                                border.color: productDetails.borderColor
                                radius: productDetails.radius
                            }                            
                        }
                    }                    
                    // ListItem to "listings" table
                    //Button {
                    //    id: listProductButton
                    //}
                }
                
                //ColumnLayout {
                //    id: inventoryManager // inventory can be managed here and sorted too
                //}    
            }
            
            Item {
                id: customerOrdersTab
                // TODO: order status: pending, unpaid, delivered/completed, refunded, etc.
            }            
        }
    }
}
