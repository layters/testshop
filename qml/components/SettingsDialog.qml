import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12

import "../../fonts/FontAwesome"//import FontAwesome 1.0
import "." as NeroshopComponents
// This page provides an interface for modifying the configuration file, "settings.lua"
// Stuff like themes, preffered currency, language, etc. will go here and will not be associated with an account but with the application config itself. This is to preserve privacy and reduce the size of the database
// todo: use previous settings as placeholderText and rewrite settings.lua
// default monero nodes cannot be modified but user may add addtional nodes to the nodelist

Popup {
    id: settingsDialog
    visible: false
    property bool hideTabText: false
    property alias theme: themeBox
    background: Rectangle {
        implicitWidth: 700
        implicitHeight: 500
        color: NeroshopComponents.Style.getColorsFromTheme()[2]
        border.color: "white"
        //DragHandler { target: settingsDialog }
        
        Rectangle {
            id: titleBar
            color: "#323232"
            height: 40
            width: parent.width
            
            Button {
                id: closeButton
                width: 25//20
                height: this.width

                anchors.verticalCenter: titleBar.verticalCenter
                anchors.right: titleBar.right
                anchors.rightMargin: 10
                text: qsTr(FontAwesome.xmark)
                contentItem: Text {  
                    text: closeButton.text
                    color: "#ffffff"
                    font.bold: true
                    font.family: FontAwesome.fontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: "#ff4d4d"
                    radius: 100
                }
                onClicked: {
                    settingsDialog.close()
                }
            }
        }
    }
    
        TabBar {
            id: settingsBar
            anchors.top: parent.top
            anchors.topMargin: titleBar.height
            anchors.horizontalCenter: parent.horizontalCenter
            property string buttonColor: "#030380"
            background: Rectangle { color: "transparent" } // hide white corners when tabButton radius is set
            
            TabButton { 
                id: generalSettingsButton
                text: qsTr("General")
                width: implicitWidth + 20
                onClicked: settingsStack.currentIndex = 0
                //display: (hideTabText) ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                //icon.source: "file:///" + neroshopResourcesDir + "/cog.png"//"/tools.png"
                checkable: true
                checked: (settingsStack.currentIndex == 0)                
                background: Rectangle {
                    color: (parent.checked) ? settingsBar.buttonColor : "#ffffff"
                    radius: 3
                }
                // This will remove the icon :(
                contentItem: Text {
                    text: parent.text
                    color: (parent.checked) ? "#e0e0e0" : "#353637"//"#000000" : "#ffffff"
                    horizontalAlignment: Text.AlignHCenter//anchors.horizontalCenter: parent.horizontalCenter
                    verticalAlignment: Text.AlignVCenter                    
                    font.bold: (parent.checked) ? true : false
                    //font.family: FontAwesome.fontFamily
                }                
            }
            
            TabButton { 
                text: (hideTabText) ? qsTr(FontAwesome.monero) : qsTr("%1   Monero Settings").arg(FontAwesome.monero)//
                width: implicitWidth + 20
                onClicked: settingsStack.currentIndex = 1
                display: AbstractButton.TextOnly
                checkable: true
                checked: (settingsStack.currentIndex == 1)
                background: Rectangle {
                    color: (parent.checked) ? settingsBar.buttonColor : "#ffffff"
                    radius: 3
                }
                contentItem: Text {
                    text: parent.text
                    color: (parent.checked) ? "#e0e0e0" : "#353637"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter                    
                    font.bold: (parent.checked) ? true : false
                    //font.family: FontAwesome.fontFamily
                }
            }            
        }
        /*Rectangle {
            id : decorator
            property real targetX: settingsBar.currentItem.x// - width * 2//settingsBar.currentItem.x + settingsBar.x// - width//generalSettingsButton.width// + width * 2
            anchors.top: settingsBar.bottom
            width: settingsBar.currentItem.width
            height: 2
            color: "#ffffff"//"#030380"
            NumberAnimation on x {
                duration: 200;
                to: decorator.targetX
                running: decorator.x != decorator.targetX
            }
        }*/        
    /*contentItem: */ScrollView {    
        id: scrollView
        width: parent.width; height: 385//anchors.fill: parent
        anchors.top: settingsBar.bottom
        anchors.topMargin: 10
        anchors.leftMargin: 10; anchors.rightMargin: anchors.leftMargin
        //contentWidth: configSettings.width; 
        //contentHeight: configSettings.height
        //clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        background: Rectangle { color: "transparent"; /*border.color: "blue"*/ } // todo: remove border from ScrollView
    
        StackLayout {
            id: settingsStack
            anchors.fill: parent//anchors.top: parent.top//settingsBar.bottom
        ColumnLayout {
            id: generalSettings
            //Layout.topMargin: 20 //Layout.fillWidth: true//Layout.alignment//anchors.fill: parent
            //spacing: 10
            GroupBox {
                title: qsTr("Currency")
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: "transparent"
                    border.color: "#030380"
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
                // GroupBox content goes here
                RowLayout {
                    spacing: 200 // spacing between Row items
                    Text {
                        text: qsTr("Preffered local currency:")
                    }
                    ComboBox {
                        //editable: true; selectTextByMouse: true
                        model: ["USD", "EUR", "JPY", "GBP", "CAD", "CHF", "AUD", "CNY", "SEK", "NZD", "MXN",]/*ListModel {
                            id: currencyModel
                            ListElement { text: "usd" }
                            ListElement { text: "eur" }
                            ListElement { text: "jpy" }
                        }*/
                        //implicitContentWidthPolicy: ComboBox.WidestText//ComboBox.ContentItemImplicitWidth
                        onAccepted: {
                            if (find(editText) === -1)
                                model.append({text: editText})
                        }
}
                }          
            }

            GroupBox {
                title: qsTr("Application")
                //width: scrollView.width//contentWidth // does nothing
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: "transparent"
                    border.color: "#030380"
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
                
                RowLayout {
                    Text {
                        text: qsTr("Theme:")
                    }
                    ComboBox {
                        id: themeBox
                        currentIndex: model.indexOf(NeroshopComponents.Style.themeName)//Component.onCompleted: currentIndex = model.indexOf(NeroshopComponents.Style.themeName) // Set the initial currentIndex to the index in the array containing themeName string
                        displayText: currentText
                        property string lastUsedDarkTheme: (Script.getBoolean("neroshop.generalsettings.application.theme.dark")) ? Script.getString("neroshop.generalsettings.application.theme.name") : "DefaultDark"
                        property string lastUsedLightTheme: (!Script.getBoolean("neroshop.generalsettings.application.theme.dark")) ? Script.getString("neroshop.generalsettings.application.theme.name") : "DefaultLight"
                        model: ["DefaultDark", "DefaultLight", "PurpleDust"]
                        onActivated: {
                            if(currentText == "PurpleDust") {
                                NeroshopComponents.Style.darkTheme = true
                                lastUsedDarkTheme = currentText
                            }
                            if(currentText == "DefaultDark") {
                                NeroshopComponents.Style.darkTheme = true
                                lastUsedDarkTheme = currentText
                            }
                            if(currentText == "DefaultLight") {
                                NeroshopComponents.Style.darkTheme = false
                                lastUsedLightTheme = currentText
                            }
                            displayText = currentText
                            NeroshopComponents.Style.themeName = displayText // update the actual theme (name)
                            themeSwitcher.checked = !NeroshopComponents.Style.darkTheme // update the theme switch                           
                            // NOTE:  on app launch, the theme will ALWAYS be reset back to its default unless you change the theme settings in your configuration file
                            //todo: change theme in configuration file too
                            console.log("Theme set to", currentText)
                        }
    delegate: ItemDelegate {
        width: themeBox.width
        contentItem: Text {
            text: modelData
            color: "#030380"
            font: themeBox.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: themeBox.highlightedIndex === index
    }

    indicator: Canvas {
        id: canvas
        x: themeBox.width - width - themeBox.rightPadding
        y: themeBox.topPadding + (themeBox.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: themeBox
            function onPressedChanged() { canvas.requestPaint(); }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = themeBox.pressed ? "#17a81a" : "#030380";
            context.fill();
        }
    }

    contentItem: Text {
        leftPadding: 0
        rightPadding: themeBox.indicator.width + themeBox.spacing

        text: themeBox.displayText
        font: themeBox.font
        color: themeBox.pressed ? "#17a81a" : "#030380"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        border.color: themeBox.pressed ? "#17a81a" : "#030380"
        border.width: themeBox.visualFocus ? 2 : 1
        radius: 2
    }

    popup: Popup {
        y: themeBox.height - 1
        width: themeBox.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: themeBox.popup.visible ? themeBox.delegateModel : null
            currentIndex: themeBox.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            border.color: "#030380"
            radius: 2
        }
    }                        
                    } // ComboBox       
                    // Window                
                } // RowLayout2
           } // GroupBox2        
                       GroupBox {
                title: qsTr("Language")
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: "transparent"
                    border.color: "#030380"
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
            } // GroupBox3    
            
        } // ColumnLayout (positions items vertically (up-and-down) I think, while RowLayout items are side-by-side)
        } // StackLayout
    } // ScrollView 
}
