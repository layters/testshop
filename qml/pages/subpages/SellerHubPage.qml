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
            buttonAt(0).checked = true //TODO: make "Inventory" default tab for quicker access to listing products
        }
    }
        
    ScrollView {
        id: scrollView
        width: parent.width; height: 1000//anchors.fill: parent
        anchors.top: tabBar.bottom; anchors.topMargin: tabBar.buttonHeight + 10
        //anchors.margins: 20        
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        clip: true
            
        StackLayout { // TODO: stackview (dashboard) with sections: overview (stats and analytics), inventory management (listing and delisting products), customer order management, reports, etc.
            id: dashboard
            anchors.fill: parent
            currentIndex: tabBar.checkedButtonIndex
            
            GridLayout {
                id: overviewTab
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: parent.height
                ////columnSpacing: 20
                
                RowLayout {
                    id: stats
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop// <- this is not working :/
                    property real numberTextFontSize: 24
                    property string textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#384364"
                    property real boxWidth: 250//(scrollView.width / 3) - 20//scrollView.width / statsRepeater.count
                    property real boxHeight: 110
                    property real boxRadius: 6
                    spacing: 15
                    property bool useDefaultBoxColor: true
                    property string boxColor: (NeroshopComponents.Style.darkTheme) ? "#384364" : "#ffffff"
                    // Products (listed)
                    Rectangle {
                        Layout.preferredWidth: stats.boxWidth
                        Layout.preferredHeight: stats.boxHeight
                        color: stats.useDefaultBoxColor ? stats.boxColor : "#f38989"
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
                                    color: "#4169e1"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : NeroshopComponents.Style.disabledColor
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
                                    //anchors.left: parent.left//anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                    //anchors.top: parent.top//anchors.top: parent.children[0].top; anchors.topMargin: -10//anchors.verticalCenter: parent.verticalCenter
                                }
                    
                                Text {
                                    text: "Products"
                                    //font.bold: true
                                    color: stats.textColor
                                    anchors.left: parent.children[0].left//anchors.horizontalCenter: parent.horizontalCenter//
                                    anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                }
                            }
                        }
                    }
                    // Sales
                    Rectangle {
                        Layout.preferredWidth: stats.boxWidth
                        Layout.preferredHeight: stats.boxHeight
                        color: stats.useDefaultBoxColor ? stats.boxColor : "green"
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
                                    color: "#8fbc8f"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : NeroshopComponents.Style.disabledColor
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
                                    //anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)//anchors.left: parent.children[0].right; anchors.leftMargin: parent.children[0].width//anchors.horizontalCenter: parent.horizontalCenter
                                    //anchors.top: parent.children[0].top; anchors.topMargin: -10//anchors.verticalCenter: parent.verticalCenter
                                }
                    
                                Text {
                                    text: "Sales"
                                    //font.bold: true
                                    color: stats.textColor
                                    anchors.left: parent.children[0].left//anchors.horizontalCenter: parent.horizontalCenter//
                                    anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                }
                            }
                        }
                    }
                    // Ratings/Feedback/Reputation
                    Rectangle {
                        Layout.preferredWidth: stats.boxWidth
                        Layout.preferredHeight: stats.boxHeight
                        color: stats.useDefaultBoxColor ? stats.boxColor : "gold"
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
                                    color: "#ffd700"//"#e6c200"//(NeroshopComponents.Style.darkTheme) ? "#ffffff" : NeroshopComponents.Style.disabledColor
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
                                    //anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)//anchors.left: parent.children[0].right; anchors.leftMargin: parent.children[0].width//anchors.horizontalCenter: parent.horizontalCenter
                                    //anchors.top: parent.children[0].top; anchors.topMargin: -10//anchors.verticalCenter: parent.verticalCenter
                                }
                    
                                Text {
                                    text: "Reputation"
                                    //font.bold: true
                                    color: stats.textColor
                                    anchors.left: parent.children[0].left//anchors.horizontalCenter: parent.horizontalCenter//
                                    anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                }
                            }
                        }
                    }
                } 
                // TODO: show recent orders from customers, show seller's top-selling products, show customer reviews on seller products
                //ColumnLayout {}
            } // overview tab
            
            GridLayout {
                id: inventoryTab
                
                ColumnLayout {
                // RegisterItem
                //SKU or UPC
                //Product title
                //Product description and bullet points
                //Product images
                //Search terms and relevant keywords (categories)
                // Subcategories
                // ListItem
                }
            }
            
            GridLayout {
                id: customerOrdersTab
            }            
        }
    }
}
