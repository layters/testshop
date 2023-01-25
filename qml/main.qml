// requires Qt version 5.12 (latest is 5.15 as of this writing). See https://doc.qt.io/qt-5/qt5-intro.html
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12//2.0 // (requires at least Qt 5.12 where QtQuick.Controls 1 is deprecated. See https://doc.qt.io/qt-5/qtquickcontrols-index.html#versions) // needed for built-in styles // TextField, TextArea (multi-lined TextField), TextFieldStyle//import QtQuick.Controls.Material 2.12 // Other styles: 
import QtQuick.Layouts 1.12//1.15 // The module is new in Qt 5.1 and requires Qt Quick 2.1. // RowLayout, ColumnLayout, GridLayout, StackLayout, Layout
import QtGraphicalEffects 1.12 // LinearGradient
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

import FontAwesome 1.0

//import neroshop.Wallet 1.0

import "components"
import "components" as NeroshopComponents
import "pages"

ApplicationWindow {
    id: mainWindow
    visible: true
    visibility: (Script.getNumber("neroshop.generalsettings.application.window.mode") == 1) ? "FullScreen" : "Windowed"
    title: qsTr("neroshop" + " v" + neroshopVersion)
    width: Script.getNumber("neroshop.generalsettings.application.window.width")
    height: Script.getNumber("neroshop.generalsettings.application.window.height")
    minimumWidth: 850
    minimumHeight: 500    
    color: NeroshopComponents.Style.getColorsFromTheme()[0]

    header: Rectangle {
        color: NeroshopComponents.Style.getColorsFromTheme()[1]
        height: 80//100 // width should be set automatically to the parent's width
        visible: (!pageLoader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
        
        Button {//Image { 
            id: neroshopLogoImageButton
            visible: false
            property real iconSize: 30
            icon.source: (NeroshopComponents.Style.darkTheme) ? "qrc:/images/appicons/Vector_Illustrator Files/LogoLight.svg" : "qrc:/images/appicons/Vector_Illustrator Files/LogoDark.svg"
            icon.color: icon.color
            icon.width: iconSize; icon.height: iconSize
            display: AbstractButton.IconOnly
            hoverEnabled: true
            background: Rectangle {
                color: "transparent"
                radius: 5
                border.color: parent.hovered ? "#ffffff" : "transparent"
            }
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            //width: 35; height: this.width
            onClicked: { 
                navBar.uncheckAllButtons()
                pageLoader.source = "qrc:/qml/pages/HomePage.qml" 
            }
        }
        
        NeroshopComponents.SearchBar {
            id: searchBar
            anchors.left: (neroshopLogoImageButton.visible) ? neroshopLogoImageButton.right : parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top; anchors.topMargin: 20        
        }    

        NeroshopComponents.NavigationalBar {
            id: navBar
            anchors.left: parent.right
            anchors.leftMargin: (-this.width - 20)
            anchors.top: parent.top
            anchors.topMargin: 20        
        }
    }
    
    Loader {
        id: pageLoader
        anchors.fill: parent
        source: "qrc:/qml/pages/MainPage.qml"
        //source: "qrc:/qml/pages/HomePage.qml"
        //source: "qrc:/qml/pages/CartPage.qml"
        //source: "qrc:/qml/pages/CatalogPage.qml"
        //source: "qrc:/qml/pages/ProductPage.qml"
        //source: "qrc:/qml/pages/OrderCheckoutPage.qml"
        ////source: "qrc:/qml/pages/ProfilePage.qml"
        ////source: "qrc:/qml/pages/Page.qml"

        onSourceChanged: {
            console.log(source);
            if (pageLoader.status == Loader.Ready) console.log('Loaded') 
            else console.log('Not Loaded')
        }
    }
    
    NeroshopComponents.SettingsDialog {
        id: settingsDialog
        anchors.centerIn: Overlay.overlay//parent: Overlay.overlay; anchors.centerIn: parent
        dim: true      
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }
    }    

    NeroshopComponents.MessageBox {////MessageDialog {
        id: messageBox
        title: "message"
        x: mainWindow.x + (mainWindow.width - this.width) / 2
        y: mainWindow.y + (mainWindow.height - this.height) / 2
    }

    NeroshopComponents.MessageBox {////MessageDialog {
        id: monerodMessageBox
        title: "prompt"
        x: mainWindow.x + (mainWindow.width - this.width) / 2
        y: mainWindow.y + (mainWindow.height - this.height) / 2
        acceptButton.visible: true
        acceptButton.onClicked: {
            console.log("Now connecting to a local node ...")
            Wallet.daemonConnect();//(moneroDaemonRpcLoginUser.text, moneroDaemonRpcLoginPwd.text)
            monerodMessageBox.close();
        }        
    }        
    // navigating between different pages: https://stackoverflow.com/a/15655043
    // The footer item is positioned to the bottom, and resized to the width of the window
    // Custom ToolBar
    footer: Rectangle {
        height: 40//; width: parent.width// width is automatically set to parent's width by default so no need to manually set the footer width
        color: NeroshopComponents.Style.getColorsFromTheme()[1]
        
        Row {
            anchors.fill: parent//anchors.horizontalCenter: parent.horizontalCenter
            anchors.rightMargin: 20 // use leftMargin only if using layoutDirection is Qt.LeftToRight
            spacing: 20 // Spacing between each Row item
            layoutDirection: Qt.RightToLeft
            
            Rectangle {
                width: themeSwitcher.width + settingsButton.width
                height: footer.height//themeSwitcher.height
                color: "transparent"
                //border.color: "blue"                
                
                NeroshopComponents.ThemeSwitch {
                    id: themeSwitcher
                    width: 40
                    anchors.left: settingsButton.right
                }

                Button {
                    id: settingsButton
                    display: AbstractButton.IconOnly
                    checkable: true
                    checked: settingsDialog.visible
                    icon.source: "qrc:/images/cog.png"//"/tools.png"
                    icon.color: (NeroshopComponents.Style.darkTheme) ? "#8fa4ff" : "#001677"////hovered ? "#001677" : "#ffffff"//(!checked && hovered) ? "#001677" : "#ffffff"
                    //icon.width: parent.width//footer.height
                    //icon.height: parent.height//32//footer.height
                    hoverEnabled: true
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: "transparent"//(parent.checked) ? "#001677" : "transparent"
                        radius: 3
                        border.color: parent.hovered ? parent.icon.color : this.color//"#001677"
                        //border.width: (!parent.checked && parent.hovered) ? 1 : 0
                    }              
                    onClicked: {
                        settingsDialog.open()
                    }
                    NeroshopComponents.Hint {
                        visible: parent.hovered
                        x: parent.x + (parent.width - this.width) / 2 // Popups don't have anchors :(
                        height: contentHeight + 20; width: contentWidth + 20
                        bottomMargin : footer.height + 5
                        text: qsTr("Settings")
                        pointer.visible: false
                    }                    
                }
            }
            
            Rectangle {
                width: daemonSyncBar.width
                height: footer.height
                color: "transparent"
                //border.color: "red"
                                
                NeroshopComponents.ProgressBar {
                    id: daemonSyncBar
                    radius: 20
                    foregroundColor: "#564978"
                    backgroundColor: "#d9d9d9"
                    //textObject.visible: true
                    hoverEnabled: true
                    anchors.verticalCenter: parent.verticalCenter//anchors.top: parent.top; anchors.topMargin: (parent.height - this.height) / 2 // center vertically on footer (height)
                    value: 0.5 // placeholder value
                    barWidth: 200

                    NeroshopComponents.Hint {
                        visible: parent.hovered
                        x: parent.x + (parent.width - this.width) / 2 // Popups don't have anchors :(
                        height: contentHeight + 20; width: (contentWidth > parent.width) ? 300 : parent.width
                        bottomMargin : footer.height + 5
                        text: qsTr("neromon\n%1 %2").arg((parent.value <= 0.0) ? "Disconnected" : ((parent.value > 0.0 && parent.value < 1.0) ? "Synchronizing" : "Connected")).arg((parent.value > 0.0 && parent.value < 1.0) ? ("(" + (parent.value * 100).toString() + "%)") : "")
                        pointer.visible: false
                    }
                }      
            }
            Rectangle {
                width: moneroDaemonSyncBar.width
                height: footer.height
                color: "transparent"
                //border.color: "plum"
                                
                NeroshopComponents.ProgressBar {
                    id: moneroDaemonSyncBar
                    radius: daemonSyncBar.radius
                    foregroundColor: NeroshopComponents.Style.moneroOrangeColor
                    backgroundColor: daemonSyncBar.backgroundColor//(NeroshopComponents.Style.darkTheme) ? "#8c8c8c" : NeroshopComponents.Style.moneroGrayColor
                    //textObject.visible: true
                    //textObject.text: "wallet sync: " + (this.value * 100).toString() + "%"
                    //textObject.color: "#ffffff"
                    hoverEnabled: true
                    anchors.verticalCenter: parent.verticalCenter//anchors.top: parent.top; anchors.topMargin: (parent.height - this.height) / 2
                    ////value: Wallet.opened ? Wallet.getSyncPercentage() : 0.0 // this does not work (fails to update value so we use Timer instead)
                    barWidth: daemonSyncBar.barWidth
                    property string daemonAddress: "monerod"//(Wallet.opened) ? settingsDialog.moneroNodeAddress : "monerod"
                    Timer {
                        interval: 1 // trigger every x miliseconds
                        running: true
                        repeat: true // If repeat is true the timer is triggered repeatedly at the specified interval
                        onTriggered: {
                            moneroDaemonSyncBar.value = Wallet.getSyncPercentage()
                        }
                    }                    
                
                    NeroshopComponents.Hint {
                        visible: parent.hovered
                        x: parent.x + (parent.width - this.width) / 2
                        height: contentHeight + 20; width: (contentWidth > parent.width) ? 300 : parent.width
                        bottomMargin : footer.height + 5
                        text: qsTr("%1\n%2 %3%4").arg(moneroDaemonSyncBar.daemonAddress).arg((!Wallet.opened || parent.value <= 0.0) ? "Disconnected" : ((parent.value > 0.0 && parent.value < 1.0) ? Wallet.getSyncMessage() : "Connected")).arg((parent.value > 0.0 && parent.value != 1.0) ? ("(" + (parent.value * 100).toFixed(2) + "%)") : "").arg((!Wallet.opened || parent.value <= 0.0) ? "" : ((parent.value > 0.0 && parent.value != 1.0) ? ("\nBlocks remaining: " + Wallet.getSyncHeight() + " / " + Wallet.getSyncEndHeight()) : ""))
                        pointer.visible: false
                    }                
                }
            }
            //Rectangle {
            //}            
        }
        /*Row {
            width: priceDisplayText.contentWidth
            height: footer.height
            anchors.left: footer.left
            anchors.leftMargin: 20
            anchors.top: footer.top
            anchors.topMargin: 0*/
            Rectangle {
                anchors.left: footer.left
                anchors.leftMargin: 20
                width: childrenRect.width////(priceChangePercentageText.visible) ? priceDisplayText.contentWidth + priceChangePercentageText.contentWidth + 10 : priceDisplayText.contentWidth // 10 is the spacing between the price and the price change percentage
                height: footer.height
                color: "transparent"
                //border.color: NeroshopComponents.Style.moneroOrangeColor
                
                Text {
                    id: priceDisplayText
                    property string scriptCurrency: Script.getString("neroshop.generalsettings.currency")
                    property string currency: Backend.isSupportedCurrency(scriptCurrency) ? scriptCurrency : "usd"
                    property double price: Backend.getXmrPrice(currency)
                    text: qsTr(/*1.toString() + " " + */FontAwesome.monero + "  %1%2").arg(Backend.getCurrencySign(currency)).arg(price.toFixed(Backend.getCurrencyDecimals(currency)))////.arg(currency.toUpperCase())
                    color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Text {
                    id: priceChangePercentageText
                    property double percent: 0.00
                    visible: false // hide this for now until I can get the actual price change percent value
                    text: qsTr("%1%2%").arg((percent >= 0) ? "+" : "").arg(percent.toFixed(2))
                    color: (percent >= 0) ? "green" : "red"
                    anchors.right: priceDisplayText.right
                    anchors.rightMargin: -(contentWidth + 10)
                    anchors.verticalCenter: priceDisplayText.verticalCenter
                    font.pointSize: 10
                    font.bold: true
                }
                                    
                HoverHandler {
                    id: priceDisplayHoverHandler
                }
                                        
                NeroshopComponents.Hint {
                    visible: false////priceDisplayHoverHandler.hovered // <- uncomment this to make the tooltip visible on hover
                    height: contentHeight + 20; width: contentWidth + 20
                    bottomMargin : footer.height + 5
                    text: qsTr("XMR / %1").arg(priceDisplayText.currency.toUpperCase())
                    pointer.visible: false
                }                             
            }
        //}
    }    
}
// error: module "QtQuick.Layouts" version 1.15 is not installed (fix by changing the version specified in /usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Layouts/plugins.qmltypes)
// error: module "Qt.labs.platform" is not installed (fix: sudo apt install qml-module-qt-labs-platform)
// error: gtk errors on "wallet_upload_button" clicked (fix: https://forums.wxwidgets.org/viewtopic.php?t=47187)

// install: qtbase5-dev for "../cmake/Qt5/Qt5Config.cmake"
// error: /usr/lib/x86_64-linux-gnu/cmake/Qt5/Qt5Config.cmake will set Qt5_FOUND to FALSE so package "Qt5" is considered to be NOT FOUND due to Qml and Quick configuration files (/usr/lib/x86_64-linux-gnu/cmake/Qt5Qml/Qt5QmlConfig.cmake; /usr/lib/x86_64-linux-gnu/cmake/Qt5Quick/Qt5QuickConfig.cmake) not being found (fix: sudo apt install qtdeclarative5-dev)
// error: module "QtQuick" is not installed (fix: qml-module-qtquick-controls qml-module-qtquick-controls2). This will also install qml-module-qtgraphicaleffects, qml-module-qtquick-layouts, qml-module-qtquick-window2, and qml-module-qtquick2
// error: module "QtQuick.Shapes" is not installed (fix: qml-module-qtquick-shapes)
