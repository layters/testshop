// requires Qt version 5.12 (latest is 5.15 as of this writing). See https://doc.qt.io/qt-5/qt5-intro.html
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12//2.0 // (requires at least Qt 5.12 where QtQuick.Controls 1 is deprecated. See https://doc.qt.io/qt-5/qtquickcontrols-index.html#versions) // needed for built-in styles // TextField, TextArea (multi-lined TextField), TextFieldStyle//import QtQuick.Controls.Material 2.12 // Other styles: 
//import QtQuick.Controls.Material 2.0
//import QtQuick.Controls.Styles 1.4 // ApplicationWindowStyle, TextFieldStyle
import QtQuick.Layouts 1.12//1.15 // The module is new in Qt 5.1 and requires Qt Quick 2.1. // RowLayout, ColumnLayout, GridLayout, StackLayout, Layout
import QtQuick.Dialogs 1.3 // MessageDialog (since Qt 5.2)
import QtGraphicalEffects 1.12 // LinearGradient
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

//import neroshop.Wallet 1.0
import "qml/components"
import "qml/components" as NeroshopComponents
import "qml/pages"

ApplicationWindow {
    id: main_window
    visible: true
    title: qsTr("neroshop" + " v" + neroshopVersion)
    width: 1280
    height: 720
    minimumWidth: 850
    minimumHeight: 500    
    color: '#202020'
    ///////////////////////////
    // Global Tooltip
    /*NeroshopComponents.Hint {
        id: hint
        visible: false
    }*/
    header: Rectangle {
        color: "#2e2e2e"
        height: 100 // width should be set automatically to the parent's width
        
        NeroshopComponents.SearchBar {
            visible: (!page_loader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
        
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 20        
        }    

        NeroshopComponents.NavigationalBar {
            visible: (!page_loader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
        
            anchors.left: parent.right
            anchors.leftMargin: (-this.width - 20)
            anchors.top: parent.top
            anchors.topMargin: 20        
        }
    }
    ///////////////////////////
    //ScrollView {
    StackView {
        anchors.fill: parent
        ////initialItem: page_loader
    }
    Loader {
        id: page_loader
        //anchors.centerIn: parent // place at center of parent // <= don't use this EVER. Not for loading pages
        anchors.fill: parent
        //source: "qrc:/qml/pages/MainPage.qml"
        //source: "qrc:/qml/pages/HomePage.qml"
        source: "qrc:/qml/pages/CatalogPage.qml"
        //source: "qrc:/qml/pages/ProductPage.qml"
        //source: "qrc:/qml/pages/OrderCheckoutPage.qml"
        ////source: "qrc:/qml/pages/Page.qml"

        onSourceChanged: {
            console.log(source);
            if (page_loader.status == Loader.Ready) console.log('Loaded') 
            else console.log('Not Loaded')
        }
    }
    //}
    // navigating between different pages: https://stackoverflow.com/a/15655043
    // The footer item is positioned to the bottom, and resized to the width of the window
    footer: ToolBar {//DialogButtonBox {
        background: Rectangle {
            color: "#2e2e2e"
        }
        
        NeroshopComponents.ThemeSwitch {
            width: 40
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

// todo: create a top banner with a dot-styled pagination (See: PageIndicator)
// todo: create a custom message box dialog in MessageBox.qml
