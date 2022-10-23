// requires Qt version 5.12 (latest is 5.15 as of this writing). See https://doc.qt.io/qt-5/qt5-intro.html
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12//2.0 // (requires at least Qt 5.12 where QtQuick.Controls 1 is deprecated. See https://doc.qt.io/qt-5/qtquickcontrols-index.html#versions) // needed for built-in styles // TextField, TextArea (multi-lined TextField), TextFieldStyle//import QtQuick.Controls.Material 2.12 // Other styles: 
//import QtQuick.Controls.Material 2.0
//import QtQuick.Controls.Styles 1.4 // ApplicationWindowStyle, TextFieldStyle
import QtQuick.Layouts 1.12//1.15 // The module is new in Qt 5.1 and requires Qt Quick 2.1. // RowLayout, ColumnLayout, GridLayout, StackLayout, Layout
import QtQuick.Dialogs 1.3 // MessageDialog (since Qt 5.2)
import QtGraphicalEffects 1.12 // LinearGradient
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

import FontAwesome 1.0

//import neroshop.Wallet 1.0

import "qml/components"
import "qml/components" as NeroshopComponents
import "qml/pages"

ApplicationWindow {
    id: mainWindow
    visible: true
    title: qsTr("neroshop" + " v" + neroshopVersion)
    width: 1280
    height: 720
    minimumWidth: 850
    minimumHeight: 500    
    color: NeroshopComponents.Style.getColorsFromTheme()[0]

    header: Rectangle {
        color: NeroshopComponents.Style.getColorsFromTheme()[1]
        height: 100 // width should be set automatically to the parent's width
        
        NeroshopComponents.SearchBar {
            id: searchBar
            visible: (!pageLoader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
        
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 20        
        }    

        NeroshopComponents.NavigationalBar {
            id: navBar
            visible: (!pageLoader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
        
            anchors.left: parent.right
            anchors.leftMargin: (-this.width - 20)
            anchors.top: parent.top
            anchors.topMargin: 20        
        }
    }
    ///////////////////////////
    //ScrollView {
    /*StackView {
        anchors.fill: parent
        ////initialItem: pageLoader
    }*/
    Loader {
        id: pageLoader
        //anchors.centerIn: parent // place at center of parent // <= don't use this EVER. Not for loading pages
        anchors.fill: parent
        source: "qrc:/qml/pages/MainPage.qml"
        //source: "qrc:/qml/pages/HomePage.qml"
        //source: "qrc:/qml/pages/CatalogPage.qml"
        //source: "qrc:/qml/pages/ProductPage.qml"
        //source: "qrc:/qml/pages/OrderCheckoutPage.qml"
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
        closePolicy: Popup.CloseOnEscape
        dim: true      
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
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
                    icon.source: "file:///" + neroshopResourcesDir + "/cog.png"//"/tools.png"
                    icon.color: "#001677"////hovered ? "#001677" : "#ffffff"//(!checked && hovered) ? "#001677" : "#ffffff"
                    //icon.width: parent.width//footer.height
                    //icon.height: parent.height//32//footer.height
                    hoverEnabled: true
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: "transparent"//(parent.checked) ? "#001677" : "transparent"
                        radius: 3
                        border.color: parent.hovered ? "#001677" : this.color//"#001677"
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
                    //textObject.visible: true
                    hoverEnabled: true
                    anchors.verticalCenter: parent.verticalCenter//anchors.top: parent.top; anchors.topMargin: (parent.height - this.height) / 2 // center vertically on footer (height)
                    value: 0.5 // placeholder value

                    NeroshopComponents.Hint {
                        visible: parent.hovered
                        x: parent.x + (parent.width - this.width) / 2 // Popups don't have anchors :(
                        height: contentHeight + 20; width: parent.width
                        bottomMargin : footer.height + 5
                        text: qsTr("neromon\n%1 (%2)").arg((parent.value < 1.0) ? "Synchronizing" : "Connected").arg((parent.value * 100).toString() + "%")
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
                    backgroundColor: NeroshopComponents.Style.moneroGrayColor
                    //textObject.visible: true
                    //textObject.text: "wallet sync: " + (this.value * 100).toString() + "%"
                    //textObject.color: "#ffffff"
                    hoverEnabled: true
                    anchors.verticalCenter: parent.verticalCenter//anchors.top: parent.top; anchors.topMargin: (parent.height - this.height) / 2
                    ////value: Wallet.isGenerated() ? Wallet.getSyncPercentage() : 0.0 // this does not work (fails to update value so we use Timer instead)     
                    Timer {
                        interval: 1 // trigger every x miliseconds
                        running: true // starts the timer, If set to true
                        repeat: true // If repeat is true the timer is triggered repeatedly at the specified interval
                        ////triggeredOnStart: false
                        onTriggered: {
                            moneroDaemonSyncBar.value = Wallet.getSyncPercentage()//Math.floor(Math.random() * (moneroDaemonSyncBar.to - moneroDaemonSyncBar.from + 1) + moneroDaemonSyncBar.from);
                        }
                    }                    
                
                    NeroshopComponents.Hint {
                        visible: parent.hovered
                        x: parent.x + (parent.width - this.width) / 2
                        height: contentHeight + 20; width: parent.width
                        bottomMargin : footer.height + 5
                        text: qsTr("%1\n%2 (%3)\n Blocks remaining: %4 / %5").arg("monerod").arg(!Wallet.isGenerated() ? "Disconnected" : ((parent.value < 1.0) ? Wallet.getSyncMessage() : "Connected")).arg((parent.value * 100).toFixed(2) + "%").arg(!Wallet.isGenerated() ? 0 : Wallet.getSyncHeight()).arg(!Wallet.isGenerated() ? 0 : Wallet.getSyncEndHeight()) // If connected to a remote node, "monerod" will be replaced by the <ip>:<port> of the remote node
                        pointer.visible: false
                    }                
                }
            }
            //Rectangle {
            //}            
        }
    }    
}
// error: module "QtQuick.Layouts" version 1.15 is not installed (fix by changing the version specified in /usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Layouts/plugins.qmltypes)
// error: module "Qt.labs.platform" is not installed (fix: sudo apt install qml-module-qt-labs-platform)
// error: gtk errors on "wallet_upload_button" clicked (fix: https://forums.wxwidgets.org/viewtopic.php?t=47187)

// install: qtbase5-dev for "../cmake/Qt5/Qt5Config.cmake"
// error: /usr/lib/x86_64-linux-gnu/cmake/Qt5/Qt5Config.cmake will set Qt5_FOUND to FALSE so package "Qt5" is considered to be NOT FOUND due to Qml and Quick configuration files (/usr/lib/x86_64-linux-gnu/cmake/Qt5Qml/Qt5QmlConfig.cmake; /usr/lib/x86_64-linux-gnu/cmake/Qt5Quick/Qt5QuickConfig.cmake) not being found (fix: sudo apt install qtdeclarative5-dev)
// error: module "QtQuick" is not installed (fix: qml-module-qtquick-controls qml-module-qtquick-controls2). This will also install qml-module-qtgraphicaleffects, qml-module-qtquick-layouts, qml-module-qtquick-window2, and qml-module-qtquick2
// error: module "QtQuick.Shapes" is not installed (fix: qml-module-qtquick-shapes)
