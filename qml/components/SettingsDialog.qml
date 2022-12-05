import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

import FontAwesome 1.0
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
    property alias currency: currencyBox
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

    FileDialog {
        id: moneroDaemonFileDialog
        fileMode: FileDialog.OpenFile
        currentFile: monerodLocationField.text // currentFile is deprecated since Qt 6.3. Use selectedFile instead
        folder: (isWindows) ? StandardPaths.writableLocation(StandardPaths.DocumentsLocation) + "/neroshop" : StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"
        nameFilters: ["Executable files (*.exe *.AppImage *)"]
    }   
    
    FolderDialog {
        id: moneroDataDirFolderDialog
        currentFolder: moneroDataDirField.text
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
                //icon.source: "file:///" + neroshopResourcesDirPath + "/cog.png"//"/tools.png"
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
                        id: currencyBox
                        currentIndex: model.indexOf(Script.getString("neroshop.generalsettings.currency").toUpperCase())
                        displayText: currentText
                        ////property string lastCurrencySet: (Script.getString("neroshop.generalsettings.currency")) ? Script.getString("neroshop.generalsettings.currency") : "USD"
                        //editable: true; selectTextByMouse: true
                        model: Backend.getCurrencyList()
                        //implicitContentWidthPolicy: ComboBox.WidestText//ComboBox.ContentItemImplicitWidth
                        onAccepted: {
                            if (find(editText) === -1)
                                model.append({text: editText})
                        }
                        
                        onActivated: {    
                            displayText = currentText
                            priceDisplayText.currency = displayText
                            priceDisplayText.price = Backend.getPrice(priceDisplayText.amount, priceDisplayText.currency)
                            priceDisplayText.text = qsTr(FontAwesome.monero + "  %1%2").arg(Backend.getCurrencySign(priceDisplayText.currency)).arg(priceDisplayText.price.toFixed(Backend.getCurrencyDecimals(priceDisplayText.currency)))
                            ////lastCurrencySet = currentText
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
        
        // Monero settings
            GridLayout {
                id: moneroSettings
                Layout.minimumWidth: parent.width - 10 // 10 is the scrollView's right margin
                            
                    
                Frame { //GroupBox {
                    //title: qsTr("Select a node type")
                    Layout.row: 0
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500//250////Layout.fillWidth: true
                    Layout.preferredHeight: contentHeight + (contentHeight / 2)//implicitContentHeight
                    /*label: Label {
                        text: parent.title
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                    }*/
                    background: Rectangle {
                        radius: 3
                        color: "transparent"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    }
                    
                    RowLayout {
                        spacing: (contentWidth + remoteNodeLabel.contentWidth)//20//100
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.fill: parent
                                        
                        ButtonGroup { 
                            id: nodeTypeGroup 
                            onClicked: { console.log("Selected condition:", button.text) }
                        }

                        RadioButton {
                            id: remoteNodeButton
                            checked: true
                            ButtonGroup.group: nodeTypeGroup
                            Label {
                                id: remoteNodeLabel
                                anchors.left: parent.right
                                anchors.leftMargin: 0
                                anchors.verticalCenter: parent.verticalCenter
                                text: FontAwesome.cloud//"Remote node"
                                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                font.bold: true
                                //font.family: FontAwesome.fontFamily
                                font.pointSize: 15
                            }                               
                        }

                        RadioButton {
                            id: localNodeButton
                            checked: false
                            ButtonGroup.group: nodeTypeGroup
                            Label {
                                anchors.left: parent.right
                                anchors.leftMargin: 0
                                anchors.verticalCenter: parent.verticalCenter
                                text: FontAwesome.house//"Local node"
                                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                                font.bold: true
                                font.family: FontAwesome.fontFamily
                                font.pointSize: 15
                            }                            
                        }                                                
                    }               
                } // GroupBox or Frame         
                
                TextField {
                    id: monerodLocationField
                    Layout.row: 1
                    Layout.alignment: Qt.AlignHCenter
                    //Layout.fillWidth: true
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50
                    placeholderText: qsTr("monerod"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    selectByMouse: true
                    text: moneroDaemonFileDialog.file
                
                    background: Rectangle { 
                        color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        radius: 3
                    }       
                    // 
                    rightPadding: 15 + moneroDaemonFileSelectionButton.width
                    // 
                    Button {
                        id: moneroDaemonFileSelectionButton
                        text: FontAwesome.ellipsis
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        implicitWidth: 32; implicitHeight: 24
                        hoverEnabled: true
                        onClicked: moneroDaemonFileDialog.open()
                        background: Rectangle {
                            color: NeroshopComponents.Style.moneroGrayColor
                            radius: 5
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.bold: true
                        }
                    }                                 
                }
                
                TextField {
                    id: moneroDataDirField
                    Layout.row: 2
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 50
                    property string defaultMoneroDataDirFolder: (isWindows) ? StandardPaths.writableLocation(StandardPaths.AppDataLocation) + "/bitmonero" : StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/.bitmonero" // C:/ProgramData/bitmonero (Windows) or ~/.bitmonero (Linux and Mac OS X)
                    placeholderText: qsTr(defaultMoneroDataDirFolder); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    selectByMouse: true
                    text: moneroDataDirFolderDialog.folder
                
                    background: Rectangle { 
                        color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                        border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        radius: 3
                    }      
                    rightPadding: 15 + moneroDataDirFolderSelectionButton.width
                    Button {
                        id: moneroDataDirFolderSelectionButton
                        text: FontAwesome.ellipsis
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        implicitWidth: 32; implicitHeight: 24
                        hoverEnabled: true
                        onClicked: moneroDataDirFolderDialog.open()
                        background: Rectangle {
                            color: NeroshopComponents.Style.moneroGrayColor
                            radius: 5
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.bold: true
                        }
                    }                                  
                }
                
                Rectangle {
                    Layout.row: 3
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500////Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    radius: 3
                    color: "transparent"
                    //border.color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"         
                    
                    RowLayout {
                        anchors.fill: parent
                                       
                        /*Label {
                            id: networkTypeLabel
                            text: qsTr("Network Type")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            Layout.alignment: Qt.AlignLeft
                            Layout.leftMargin: 5//25
                        }*/
                        
                        ComboBox {
                            id: moneroNetworkTypeBox
                            model: ["mainnet", "stagenet", "testnet"]
                            currentIndex: model.indexOf(Script.getString("neroshop.monero.daemon.network_type").toLowerCase())
                            displayText: currentText
                            Layout.fillWidth: true/*Layout.alignment: Qt.AlignRight
                            Layout.rightMargin: 5*/
                        
                            onActivated: {    
                                displayText = currentText
                                // = displayText
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.row: 4
                    Layout.alignment: Qt.AlignHCenter
                    
                    TextField {
                        id: moneroIPField
                        ////Layout.row: 4
                        ////Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 250////500
                        Layout.preferredHeight: 50
                        placeholderText: qsTr(/*(confirmExternalBindSwitch.checked) ? "0.0.0.0" : */Script.getString("neroshop.monero.daemon.ip")); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }               
                    }
                
                    TextField {
                        id: moneroPortField
                        ////Layout.row: 5
                        ////Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 245 // Default row spacing is 5 so this is reduced by 5////500
                        Layout.preferredHeight: 50
                        //property string portByNetworkType: {}
                        placeholderText: qsTr(Script.getString("neroshop.monero.daemon.port")); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }     
                    }          
                }                  
                /*Row {
                    NeroshopComponents.Switch {
                        id: confirmExternalBindSwitch
                        Layout.row: 5
                    }
                }
                Row {
                    NeroshopComponents.Switch {
                        id: restrictedRpcSwitch
                        Layout.row: 6
                    }                
                }
                //ListView {}
                //    id: remoteNodeList
                //}
                // manage daemon buttons: stop daemon, start daemon, etc.
                //Button {
                    //id:saveSettingsButton
                //}
                */                
            } // moneroSettings
        } // StackLayout
    } // ScrollView 
}
