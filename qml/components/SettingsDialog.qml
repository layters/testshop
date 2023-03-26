import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

import neroshop.CurrencyExchangeRates 1.0
import FontAwesome 1.0
import "." as NeroshopComponents
// This page provides an interface for modifying the configuration file, "settings.lua"
// Stuff like themes, preferred currency, language, etc. will go here and will not be associated with an account but with the application config itself. This is to preserve privacy and reduce the size of the database
// todo: use previous settings as placeholderText and rewrite settings.lua
// default monero nodes cannot be modified but user may add addtional nodes to the nodelist

Popup {
    id: settingsDialog
    visible: false
    modal: true//clip: true
    closePolicy: Popup.CloseOnEscape    
    property bool hideTabText: false
    property alias currentIndex: settingsStack.currentIndex
    // General tab properties
    property alias theme: themeBox
    property alias currency: currencyBox
    property alias hideHomepageButton: hideHomepageButtonSwitch.checked
    property alias hidePriceDisplay: priceDisplaySwitch.checked
    // Wallet settings
    property alias balanceDisplay: balanceDisplayBox.currentIndex//property alias balanceDisplay: balanceDisplayBox.currentText
    property alias balanceAmountPrecision: balancePrecisionBox.currentText
    property alias showCurrencySign: showCurrencySignSwitch.checked
    property alias blockExplorer: blockExplorerBox.currentText
    // Catalog settings
    property alias catalogPriceBox: priceDisplayBox
    property alias hideProductDetails: hideProductDetailsSwitch.checked
    property alias gridDetailsAlignCenter: gridDetailsAlignCenterSwitch.checked
    property alias catalogViewBox: viewBox
    ////property alias hideIllegalProducts: hideIllegalProductsSwitch.checked
    // Monero tab properties
    property alias moneroNodeType: nodeTypeStackLayout.currentIndex//nodeTypeGroup.checkedButton.stackLayoutIndex
    property string moneroNodeAddress: (nodeTypeStackLayout.currentIndex == remoteNodeButton.stackLayoutIndex) ? moneroRemoteNodeList.selectedNode.replace(/^(https?:|)\/\//, '') : (moneroDaemonIPField.placeholderText + ":" + moneroDaemonPortField.placeholderText)
    property string moneroNodeDefaultPort: moneroDaemonPortField.placeholderText
    property string monerodPath: Backend.urlToLocalFile(monerodPathField.text)
    property string moneroDataDir: (moneroDataDirField.text.length < 1) ? Backend.urlToLocalFile(moneroDataDirField.placeholderText) : Backend.urlToLocalFile(moneroDataDirField.text)
    property bool confirmExternalBind: confirmExternalBindSwitch.checked
    property bool restrictedRpc: restrictedRpcSwitch.checked
    property string moneroDaemonUsername: moneroDaemonRpcLoginUser.text
    property string moneroDaemonPassword: moneroDaemonRpcLoginPwd.text
    property bool moneroDaemonAutoSync: (nodeTypeStackLayout.currentIndex == remoteNodeButton.stackLayoutIndex) ? autoNodeSyncSwitch.checked : autoDaemonSyncSwitch.checked
    property Button moneroDaemonConnectButton: (nodeTypeStackLayout.currentIndex == remoteNodeButton.stackLayoutIndex) ? remoteNodeConnectButton : localNodeConnectButton
  
    function resetScrollBar() {
        scrollView.ScrollBar.vertical.position = 0.0
    }
    
    function getPortByNetworkType() {
        let network_type = Wallet.getNetworkTypeString()
        if(network_type == "mainnet") return "18081";
        if(network_type == "testnet") return "28081";
        if(network_type == "stagenet") return "38081";
        return "18081"
    }
    
    background: Rectangle {
        implicitWidth: 700
        implicitHeight: 500
        color: NeroshopComponents.Style.getColorsFromTheme()[2]
        ////border.color: "white"//; border.width: 1
        radius: 8
        //DragHandler { target: settingsDialog }   
        
        Rectangle {
            id: titleBar
            color: "#323232"
            height: 40//settingsDialog.topPadding
            width: parent.width
            anchors.left: parent.left
            anchors.right: parent.right
            radius: 6
            // Rounded top corners??
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: parent.height / 2
                color: parent.color
            }
            
            Label {
                text: "Settings"
                color: "#ffffff"
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
            
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
            property string buttonOnColor: "#030380"
            property string buttonOffColor: "transparent"//"#ffffff"
            background: Rectangle { color: "transparent" } // hide white corners when tabButton radius is set
            property real buttonRadius: 3
            
            TabButton { 
                id: generalSettingsButton
                text: qsTr("General")
                width: implicitWidth + 20
                onClicked: {
                    settingsStack.currentIndex = 0
                    resetScrollBar()
                }
                //display: (hideTabText) ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                //icon.source: "file:///" + neroshopResourcesDirPath + "/cog.png"//"/tools.png"
                checkable: true
                checked: (settingsStack.currentIndex == 0)                
                background: Rectangle {
                    color: (parent.checked) ? settingsBar.buttonOnColor : settingsBar.buttonOffColor
                    radius: settingsBar.buttonRadius//;border.color: parent.hovered ? settingsBar.buttonOnColor : settingsBar.buttonOffColor
                }
                // This will remove the icon :(
                contentItem: Text {
                    text: parent.text
                    color: (parent.checked) ? "#ffffff" : ((NeroshopComponents.Style.darkTheme) ? "#e0e0e0" : "#353637")//"#000000" : "#ffffff"
                    horizontalAlignment: Text.AlignHCenter//anchors.horizontalCenter: parent.horizontalCenter
                    verticalAlignment: Text.AlignVCenter                    
                    font.bold: true//(parent.checked) ? true : false
                    //font.family: FontAwesome.fontFamily
                }         
                MouseArea {
                    anchors.fill: parent
                    onPressed: mouse.accepted = false
                    cursorShape: !parent.checked ? Qt.PointingHandCursor : Qt.ArrowCursor
                }       
            }
            
            TabButton { 
                text: (hideTabText) ? qsTr(FontAwesome.monero) : qsTr("Node")//.arg(FontAwesome.monero)
                width: implicitWidth + 20
                onClicked: {
                    settingsStack.currentIndex = 1
                    resetScrollBar()
                }
                display: AbstractButton.TextOnly
                checkable: true
                checked: (settingsStack.currentIndex == 1)
                background: Rectangle {
                    color: (parent.checked) ? settingsBar.buttonOnColor : settingsBar.buttonOffColor
                    radius: settingsBar.buttonRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: (parent.checked) ? "#ffffff" : ((NeroshopComponents.Style.darkTheme) ? "#e0e0e0" : "#353637")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter                    
                    font.bold: true
                    //font.family: FontAwesome.fontFamily
                }
                MouseArea {
                    anchors.fill: parent
                    onPressed: mouse.accepted = false
                    cursorShape: !parent.checked ? Qt.PointingHandCursor : Qt.ArrowCursor
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
    ScrollView {    
        id: scrollView
        width: parent.width; height: 385//anchors.fill: parent
        anchors.top: settingsBar.bottom
        anchors.topMargin: 10
        anchors.leftMargin: 10; anchors.rightMargin: anchors.leftMargin
        //contentWidth: configSettings.width; 
        //contentHeight: configSettings.height
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded//ScrollBar.AlwaysOn
        background: Rectangle { color: "transparent"; /*border.color: "blue"*/ } // todo: remove border from ScrollView
    
        StackLayout {
            id: settingsStack
            anchors.fill: parent//anchors.top: parent.top//settingsBar.bottom
            
            property real contentBoxWidth: 600
            property string contentBoxColor: "transparent"
            property string contentBoxBorderColor: (NeroshopComponents.Style.darkTheme) ? "#f0f0f0" : "#4d4d4d"//"#030380"
            property real comboBoxWidth: 300
            property real comboBoxButtonWidth: 50
            property bool comboBoxNestedButton: true
            
        ColumnLayout {
            id: generalSettings
            Layout.preferredWidth: parent.width//Layout.minimumWidth: parent.width - 10 // 10 is the scrollView's right margin
            //Layout.topMargin: 20 //Layout.fillWidth: true//Layout.alignment//anchors.fill: parent
            spacing: 30
            GroupBox {
                //Layout.row: 0
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: settingsStack.contentBoxWidth
                title: qsTr("Currency")
                
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: settingsStack.contentBoxColor
                    border.color: settingsStack.contentBoxBorderColor
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
                ColumnLayout {
                    id: currencyColumn
                    width: parent.width; height: childrenRect.height
                    //spacing: 200 // spacing between Row items
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: currencyBox.verticalCenter
                            text: qsTr("Preferred local currency:")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            //Layout.alignment: Qt.AlignLeft; Layout.leftMargin: 0
                        }
                      
                        NeroshopComponents.ComboBox {
                            id: currencyBox
                            anchors.right: parent.right//Layout.alignment: Qt.AlignRight; Layout.rightMargin: 0
                            width: settingsStack.comboBoxWidth
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
                                ////lastCurrencySet = currentText
                            }
                            indicatorWidth: settingsStack.comboBoxButtonWidth
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                            color: "#f2f2f2"//(NeroshopComponents.Style.darkTheme) ? "#101010" : "#f0f0f0"
                            //textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        }
                    }
                    // Price API - TODO
                    /*Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: priceApiBox.verticalCenter
                            text: qsTr("Price API:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.ComboBox {
                            id: priceApiBox
                            anchors.right: parent.right
                            width: settingsStack.comboBoxWidth; indicatorWidth: settingsStack.comboBoxButtonWidth
                            //model: ["CoinGecko", "CoinMarketCap"]
                            Component.onCompleted: currentIndex = find("CoinGecko")
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                            color: "#f2f2f2"
                        }
                    }*/
                }          
            }

            GroupBox {
                //Layout.row: 1
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: settingsStack.contentBoxWidth
                title: qsTr("Application")
                //width: scrollView.width//contentWidth // does nothing
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: settingsStack.contentBoxColor
                    border.color: settingsStack.contentBoxBorderColor
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
                
                ColumnLayout {
                    id: appColumn
                    width: parent.width; height: childrenRect.height
                    //spacing: 200 // spacing between Row items
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            text: qsTr("Theme:")
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            anchors.verticalCenter: themeBox.verticalCenter//Layout.alignment: Qt.AlignLeft; Layout.leftMargin: 0
                        }
                        NeroshopComponents.ComboBox {
                            id: themeBox
                            anchors.right: parent.right//Layout.alignment: Qt.AlignRight; Layout.rightMargin: 0
                            width: settingsStack.comboBoxWidth
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
                            indicatorWidth: settingsStack.comboBoxButtonWidth
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                            color: "#f2f2f2"
                        } // ComboBox   
                    }    
                    // Hide homepage button
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: hideHomepageButtonSwitch.verticalCenter
                            text: qsTr("Hide homepage button:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.Switch {
                            id: hideHomepageButtonSwitch
                            anchors.right: parent.right; anchors.rightMargin: 5
                            //width: settingsStack.comboBoxWidth
                            checked: true
                            radius: 13
                            backgroundCheckedColor: "#605185"
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: priceDisplaySwitch.verticalCenter
                            text: qsTr("Hide price display:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.Switch {
                            id: priceDisplaySwitch
                            anchors.right: parent.right; anchors.rightMargin: 5
                            //width: settingsStack.comboBoxWidth
                            checked: false
                            radius: 13
                            backgroundCheckedColor: "#605185"
                        }
                    }                                                    
                } // RowLayout2
           } // GroupBox2        
           GroupBox {
               //Layout.row: 2
               Layout.alignment: Qt.AlignHCenter
               Layout.preferredWidth: settingsStack.contentBoxWidth
               title: qsTr("Localization")
                
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: settingsStack.contentBoxColor
                    border.color: settingsStack.contentBoxBorderColor
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
            ColumnLayout {
                    id: languageColumn
                    width: parent.width; height: childrenRect.height
                    //spacing: 200 // spacing between Row items
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                Label {
                    text: qsTr("Language:")
                    color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                    anchors.verticalCenter: languageBox.verticalCenter
                }

                NeroshopComponents.ComboBox {
                    id: languageBox
                    anchors.right: parent.right//Layout.alignment: Qt.AlignRight; Layout.rightMargin: 0
                    width: settingsStack.comboBoxWidth
                    currentIndex: model.indexOf("English")
                    model: ["English"] // TODO logic from controller
                    indicatorWidth: settingsStack.comboBoxButtonWidth
                    indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                    color: "#f2f2f2"
                }
                }
            }                
            } // GroupBox3  
            // Wallet settings
            GroupBox {
                title: qsTr("Wallet")
                //Layout.row: 3
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: settingsStack.contentBoxWidth
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: settingsStack.contentBoxColor
                    border.color: settingsStack.contentBoxBorderColor
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
                // TODO: balance display (show total balance, show unlocked, show all), decimals (3, 6, 12, etc.)  
                ColumnLayout {
                    id: walletSetColumn
                    width: parent.width; height: childrenRect.height
                    // Balance display
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height                        
                        Text {
                            anchors.verticalCenter: balanceDisplayBox.verticalCenter
                            text: qsTr("Balance display:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }

                        NeroshopComponents.ComboBox {
                            id: balanceDisplayBox
                            anchors.right: parent.right//; anchors.rightMargin: 0
                            width: settingsStack.comboBoxWidth; indicatorWidth: settingsStack.comboBoxButtonWidth
                            model: ["All balances", "Locked balance only", "Unlocked balance only"]
                            Component.onCompleted: currentIndex = find("All balances")
                            color: "#f2f2f2"
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                        }
                    }
                    // Precision/Decimal places
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: balancePrecisionBox.verticalCenter
                            text: qsTr("Decimals:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }

                        NeroshopComponents.ComboBox {
                            id: balancePrecisionBox
                            anchors.right: parent.right
                            width: settingsStack.comboBoxWidth; indicatorWidth: settingsStack.comboBoxButtonWidth
                            model: ["3", "6", "9", "12"]
                            Component.onCompleted: currentIndex = find("12")
                            color: "#f2f2f2"
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                        }
                    }
                    // Currency sign
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: showCurrencySignSwitch.verticalCenter
                            text: qsTr("Show currency code:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.Switch {
                            id: showCurrencySignSwitch
                            anchors.right: parent.right; anchors.rightMargin: 5
                            //width: settingsStack.comboBoxWidth
                            checked: false
                            radius: 13
                            backgroundCheckedColor: "#605185"
                        }
                    }                    
                    // Block explorer
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: blockExplorerBox.verticalCenter
                            text: qsTr("Block explorer:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.ComboBox {
                            id: blockExplorerBox
                            anchors.right: parent.right
                            width: settingsStack.comboBoxWidth; indicatorWidth: settingsStack.comboBoxButtonWidth
                            model: ["xmrchain.net"]
                            Component.onCompleted: currentIndex = find("xmrchain.net")
                            color: "#f2f2f2"
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                        }
                    }
                }
            } // Balance GroupBox
            // Hide or Show price display
            // TODO: Privacy tab: Tor, I2P settings
            // TODO: Paths selection
            // TODO: Lock on inactivity
            // Catalog settings
            GroupBox {
                title: qsTr("Catalog")
                //Layout.row: 3
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: settingsStack.contentBoxWidth
                background: Rectangle {
                    y: parent.topPadding - parent.bottomPadding
                    width: parent.width
                    height: parent.height - parent.topPadding + parent.bottomPadding
                    color: settingsStack.contentBoxColor
                    border.color: settingsStack.contentBoxBorderColor
                    radius: 2
                }
                label: Label {
                    x: parent.leftPadding
                    width: parent.availableWidth
                    text: parent.title
                    color: parent.background.border.color//"#030380"
                    elide: Text.ElideRight
                }
                // TODO: catalog display (show xmr price, show fiat price, show all)
                ColumnLayout {
                    id: catalogSetColumn
                    width: parent.width; height: childrenRect.height
                    // Catalog view display
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: viewBox.verticalCenter
                            text: qsTr("Display:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.ComboBox {
                            id: viewBox
                            anchors.right: parent.right
                            width: settingsStack.comboBoxWidth; indicatorWidth: settingsStack.comboBoxButtonWidth
                            model: ["Grid view", "List view"]
                            Component.onCompleted: currentIndex = find("Grid view")
                            color: "#f2f2f2"
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                        }
                    }                    
                    // Product details
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: hideProductDetailsSwitch.verticalCenter
                            text: qsTr("Hide product details:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.Switch {
                            id: hideProductDetailsSwitch
                            anchors.right: parent.right; anchors.rightMargin: 5
                            //width: settingsStack.comboBoxWidth
                            checked: false
                            radius: 13
                            backgroundCheckedColor: "#605185"
                        }
                    }
                    // Catalog price display
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Text {
                            anchors.verticalCenter: priceDisplayBox.verticalCenter
                            text: qsTr("Price display:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.ComboBox {
                            id: priceDisplayBox
                            anchors.right: parent.right
                            width: settingsStack.comboBoxWidth; indicatorWidth: settingsStack.comboBoxButtonWidth
                            model: ["All prices", "Fiat price only", "Monero price only"]
                            Component.onCompleted: currentIndex = find("All prices")
                            color: "#f2f2f2"
                            indicatorDoNotPassBorder: settingsStack.comboBoxNestedButton
                        }
                    }                    
                    // Grid product details aligned center
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        visible: false // hide this option as it is not necessary
                        Text {
                            anchors.verticalCenter: gridDetailsAlignCenterSwitch.verticalCenter
                            text: qsTr("Align product details at center (Grid):")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.Switch {
                            id: gridDetailsAlignCenterSwitch
                            anchors.right: parent.right; anchors.rightMargin: 5
                            //width: settingsStack.comboBoxWidth
                            checked: false
                            radius: 13
                            backgroundCheckedColor: "#605185"
                        }
                    }
                    // Show/Hide illegal products
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        visible: false // hide this option for now
                        Text {
                            anchors.verticalCenter: hideIllegalProductsSwitch.verticalCenter
                            text: qsTr("Hide illegal products:")
                            color: NeroshopComponents.Style.darkTheme ? "#ffffff" : "#000000"
                        }
                        
                        NeroshopComponents.Switch {
                            id: hideIllegalProductsSwitch
                            anchors.right: parent.right; anchors.rightMargin: 5
                            //width: settingsStack.comboBoxWidth
                            checked: true // ALWAYS hide illegal products by default!!
                            radius: 13
                            backgroundCheckedColor: "#605185"
                        }
                    }
                }
            }            
            
        } // generalSettings ColumnLayout
        
        // Monero settings
            ColumnLayout {
                id: moneroSettings
                Layout.minimumWidth: parent.width - 10 // 10 is the scrollView's right margin
                Layout.minimumHeight: 500 // Increase this value whenever more items are added inside the scrollview
                //rowSpacing:  // The default value is 5 but we'll set this later

                    
                    RowLayout {
                        //Layout.row: 0
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 500
                        //spacing: 5
                        property string checkedColor: NeroshopComponents.Style.moneroOrangeColor////"#0069d9"
                                        
                        ButtonGroup { 
                            id: nodeTypeGroup 
                            onClicked: { nodeTypeStackLayout.currentIndex = button.stackLayoutIndex }
                        }
                        
                        Rectangle {
                            color: (remoteNodeButton.checked) ? parent.checkedColor : "#ffffff"//"transparent"
                            border.color: (remoteNodeButton.checked) ? parent.checkedColor : "#989999"
                            border.width: (remoteNodeButton.checked) ? 0 : 2
                            radius: 3
                            Layout.preferredWidth: 250
                            Layout.preferredHeight: 50//Layout.fillHeight: true
                            
                        MouseArea {
                            anchors.fill: parent
                            ////cursorShape: Qt.PointingHandCursor
                            onClicked: { remoteNodeButton.checked = true 
                                nodeTypeStackLayout.currentIndex = remoteNodeButton.stackLayoutIndex
                            }
                        }

                        NeroshopComponents.RadioButton {
                            id: remoteNodeButton
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            checked: true
                            ButtonGroup.group: nodeTypeGroup
                            text: qsTr("Remote node")//FontAwesome.cloud
                            color: checked ? parent.parent.checkedColor : "#d9dada"
                            borderColor: checked ? "#ffffff" : "#d9dada"
                            textColor: checked ? "#ffffff" : "#989999"
                            textObject.font.bold: true
                            ////innercolor: borderColor
                            property int stackLayoutIndex: 0
                            //font.family: FontAwesome.fontFamily
                            //font.pointSize: 15                       
                        }
                        
                        }

                        Rectangle {
                            color: (localNodeButton.checked) ? parent.checkedColor : "#ffffff"//"transparent"
                            border.color: (localNodeButton.checked) ? parent.checkedColor : "#989999"
                            border.width: (localNodeButton.checked) ? 0 : 2
                            radius: 3
                            Layout.preferredWidth: 250 - parent.spacing
                            Layout.preferredHeight: 50//Layout.fillHeight: true
                            
                        MouseArea {
                            anchors.fill: parent
                            ////cursorShape: Qt.PointingHandCursor
                            onClicked: { localNodeButton.checked = true 
                                nodeTypeStackLayout.currentIndex = localNodeButton.stackLayoutIndex
                            }
                        }
                        
                        NeroshopComponents.RadioButton {
                            id: localNodeButton
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            checked: false
                            ButtonGroup.group: nodeTypeGroup
                            text: qsTr("Local node")//FontAwesome.house
                            color: checked ? parent.parent.checkedColor : "#d9dada"
                            borderColor: checked ? "#ffffff" : "#d9dada"
                            textColor: checked ? "#ffffff" : "#989999"
                            textObject.font.bold: true
                            ////innercolor: borderColor
                            property int stackLayoutIndex: 1                 
                        }        
                        }                                        
                    }               
                //} // GroupBox or Frame                         
                
                StackLayout {
                    id: nodeTypeStackLayout
                    //Layout.row: 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
Item {
    Layout.preferredWidth: localNodeColumn.childrenRect.width
    Layout.preferredHeight: remoteNodeColumn.childrenRect.height
    ColumnLayout {
        id: remoteNodeColumn
        anchors.horizontalCenter: parent.horizontalCenter//anchors.fill: parent
        spacing: 5
            Frame {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    Layout.preferredWidth: 500
                    Layout.preferredHeight: 300//200//Layout.fillHeight: true
                    background: Rectangle {
                        radius: 3
                        color: "transparent"
                        border.color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#7d7d97" : "#a8a8a8") : "#989999"
                        //border.width: 1
                    }

                NeroshopComponents.NodeList {
                    id: moneroRemoteNodeList
                    anchors.fill: parent                    
                }
            }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    
                    TextField {
                        id: moneroNodeIPField
                        Layout.preferredWidth: (moneroNodePortField.width * 3) - parent.spacing // Default row spacing is 5 so the width is reduced by 5
                        Layout.preferredHeight: 50
                        placeholderText: qsTr("Custom node IP address"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }               
                    }
                
                    TextField {
                        id: moneroNodePortField
                        Layout.preferredWidth: (500 / 4)
                        Layout.preferredHeight: 50
                        placeholderText: getPortByNetworkType()
                        placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }     
                    }          
                }
                
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500
                    
                    Button {
                        id: remoteNodeConnectButton
                        Layout.preferredWidth: parent.width////contentItem.contentWidth + 30
                        Layout.preferredHeight: contentItem.contentHeight + 30
                        text: qsTr("Connect")
                        property bool disabled: !Wallet.opened//{ if(!Wallet.isOpened()) return true; return Wallet.isDaemonSynced() }
                        background: Rectangle {
                            color: remoteNodeConnectButton.disabled ? NeroshopComponents.Style.moneroGrayColor : NeroshopComponents.Style.moneroOrangeColor
                            radius: 3//6
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if(remoteNodeConnectButton.disabled) return;
                            if(!Wallet.isOpened()) {messageBox.text="Wallet must be opened first before connecting to a node";messageBox.open();return;}
                            let remote_node = (moneroNodeIPField.length > 0) ? (moneroNodePortField.length > 0 ? (moneroNodeIPField.text + ":" + moneroNodePortField.text) : (moneroNodeIPField.text + ":" + moneroNodePortField.placeholderText)) : moneroRemoteNodeList.selectedNode.replace(/^(https?:|)\/\//, '')//console.log("remote_node", remote_node)
                            let remote_node_ip = remote_node.split(":")[0]
                            let remote_node_port = remote_node.split(":")[1]
                            console.log("connecting to remote node:", (remote_node_ip + ":" + remote_node_port))
                            Wallet.nodeConnect(remote_node_ip, remote_node_port)
                        }
                    }
                }                
                
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500

                    Label {
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 0
                        text: qsTr("Automatically sync to daemon on login")
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                    }
                                        
                    NeroshopComponents.Switch {
                        id: autoNodeSyncSwitch
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        checked: true
                        radius: 13
                        backgroundCheckedColor: NeroshopComponents.Style.moneroOrangeColor
                    }                
                }                            
    }
} // Item 0                    
                    
                    Item {
                        Layout.preferredWidth: localNodeColumn.childrenRect.width
                        Layout.preferredHeight: localNodeColumn.childrenRect.height
            ColumnLayout {
                id: localNodeColumn
                anchors.horizontalCenter: parent.horizontalCenter////anchors.fill: parent//Layout.fillWidth: true
                spacing: 5
                
                TextField {
                    id: monerodPathField
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500//Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    placeholderText: qsTr((isWindows) ? "monerod.exe" : "monerod"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
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

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    
                    TextField {
                        id: moneroDaemonIPField
                        Layout.preferredWidth: (moneroDaemonPortField.width * 3) - parent.spacing // Default row spacing is 5 so the width is reduced by 5
                        Layout.preferredHeight: 50
                        placeholderText: qsTr((confirmExternalBindSwitch.checked) ? "0.0.0.0" : "localhost"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                        readOnly: localNodeButton.checked
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }               
                    }
                
                    TextField {
                        id: moneroDaemonPortField
                        Layout.preferredWidth: (500 / 4)
                        Layout.preferredHeight: 50
                        placeholderText: getPortByNetworkType()
                        placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                        readOnly: localNodeButton.checked
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }     
                    }          
                }          
                        
                // Full node mainainers can require a login in order for others to connect to their daemon
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    
                    TextField {
                        id: moneroDaemonRpcLoginUser
                        Layout.preferredWidth: (500 / 2) - parent.spacing // Default row spacing is 5 so the width is reduced by 5
                        Layout.preferredHeight: 50
                        placeholderText: qsTr("Daemon Username (optional)"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }               
                    }
                
                    TextField {
                        id: moneroDaemonRpcLoginPwd
                        Layout.preferredWidth: (500 / 2)
                        Layout.preferredHeight: 50
                        placeholderText: qsTr("Daemon Password"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                        echoMode: TextInput.PasswordEchoOnEdit
                        inputMethodHints: Qt.ImhSensitiveData
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }     
                    }          
                }
                
                /*TextField {
                        id: moneroDaemonOptFlags
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 500 // Default row spacing is 5 so the width is reduced by 5
                        Layout.preferredHeight: 50
                        placeholderText: qsTr("Daemon flags (optional)"); placeholderTextColor: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        selectByMouse: true
                
                        background: Rectangle { 
                            color: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#ffffff"
                            border.color: (NeroshopComponents.Style.darkTheme) ? "#a9a9a9" : "#696969"
                            radius: 3
                        }               
                    }*/                              
                        
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500
                    
                    Label {
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 0
                        text: qsTr("Confirm external bind")
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                    }
                    
                    NeroshopComponents.Switch {
                        id: confirmExternalBindSwitch
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        checked: Script.getBoolean("neroshop.monero.daemon.confirm_external_bind");//false
                        radius: 13
                        backgroundCheckedColor: NeroshopComponents.Style.moneroOrangeColor
                    }
                }
                
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500

                    Label {
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 0
                        text: qsTr("Restricted RPC")
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                    }
                                        
                    NeroshopComponents.Switch {
                        id: restrictedRpcSwitch
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        checked: Script.getBoolean("neroshop.monero.daemon.restricted_rpc");//true
                        radius: 13
                        backgroundCheckedColor: NeroshopComponents.Style.moneroOrangeColor
                    }                
                }
                // manage daemon buttons: stop daemon, start daemon, etc.
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500
                    
                    Button {
                        Layout.preferredWidth: 250 - parent.spacing////parent.width////contentItem.contentWidth + 30
                        Layout.preferredHeight: contentItem.contentHeight + 30
                        text: qsTr("Launch")////("Launch and connect")
                        background: Rectangle {
                            color: NeroshopComponents.Style.moneroOrangeColor
                            radius: 3
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if(Backend.isWalletDaemonRunning()) { messageBox.text = "monerod is already running in the background"; messageBox.open(); return; }
                            if(monerodPathField.text.length < 1) {messageBox.text="monerod not found. Please set the path to monerod or use a remote node instead";messageBox.open();return;}
                            Wallet.daemonExecute(Backend.urlToLocalFile(monerodPathField.text), confirmExternalBindSwitch.checked, restrictedRpcSwitch.checked, (moneroDataDirField.text.length < 1) ? Backend.urlToLocalFile(moneroDataDirField.placeholderText) : Backend.urlToLocalFile(moneroDataDirField.text),
                                Wallet.getNetworkTypeString(), 0); // 0 = placeholder restore_height
                           // Connect to local node
                           /*if(!Wallet.isOpened()) return; // no need for this if we are only launching the monerod executable
                           Wallet.daemonConnect()//(moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text)*/
                        }
                    }
                    
                    Button {
                        id: localNodeConnectButton
                        Layout.preferredWidth: 250////contentItem.contentWidth + 30
                        Layout.preferredHeight: contentItem.contentHeight + 30
                        text: qsTr("Connect")
                        property bool disabled: !Wallet.opened/*{
                            return (!Backend.isWalletDaemonRunning() || !Wallet.isOpened()) // doesn't work :(
                        }*/
                        background: Rectangle {
                            color: localNodeConnectButton.disabled ? NeroshopComponents.Style.moneroGrayColor : NeroshopComponents.Style.moneroOrangeColor
                            radius: 3
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            //localNodeConnectButton.disabled = (!Backend.isWalletDaemonRunning() || !Wallet.isOpened()) // doesn't work :(
                            if(localNodeConnectButton.disabled) return; // button is disabled, exit function
                            if(!Wallet.opened) {messageBox.text="Wallet must be opened first before connecting to a node";messageBox.open();return;} // make sure wallet is opened beforehand
                            if(Wallet.isDaemonSynced()) return; // local node is already synced so there's no need to connect to it again
                            ////if(!Backend.isWalletDaemonRunning()) {messageBox.text="monerod must be launched first before connecting to it";messageBox.open();return;}
                            Wallet.daemonConnect()//(moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text)     
                        }
                    }                    
                }
                
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 500

                    Label {
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 0
                        text: qsTr("Automatically sync to daemon on login")
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                    }
                                        
                    NeroshopComponents.Switch {
                        id: autoDaemonSyncSwitch
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        checked: true
                        radius: 13
                        backgroundCheckedColor: NeroshopComponents.Style.moneroOrangeColor
                    }                
                }
                                   
                } // ColumnLayout
    FileDialog {
        id: moneroDaemonFileDialog
        fileMode: FileDialog.OpenFile
        currentFile: monerodPathField.text // currentFile is deprecated since Qt 6.3. Use selectedFile instead
        folder: (isWindows) ? StandardPaths.writableLocation(StandardPaths.DocumentsLocation) + "/neroshop" : StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"
        nameFilters: ["Executable files (*.exe *.AppImage *)"]
    }   
    
    FolderDialog {
        id: moneroDataDirFolderDialog
        currentFolder: moneroDataDirField.text
    }                
                } // Item 1
                } // StackLayout (node)             
            } // moneroSettings
        } // StackLayout (settings)
    } // ScrollView 
}
