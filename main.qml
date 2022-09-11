// requires Qt version 5.12 (latest is 5.15 as of this writing). See https://doc.qt.io/qt-5/qt5-intro.html
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12//2.0 // (requires at least Qt 5.12 where QtQuick.Controls 1 is deprecated. See https://doc.qt.io/qt-5/qtquickcontrols-index.html#versions) // needed for built-in styles // TextField, TextArea (multi-lined TextField), TextFieldStyle//import QtQuick.Controls.Material 2.12 // Other styles: 
//import QtQuick.Controls.Material 2.0
//import QtQuick.Controls.Styles 1.4 // ApplicationWindowStyle, TextFieldStyle
import QtQuick.Layouts 1.12//1.15 // The module is new in Qt 5.1 and requires Qt Quick 2.1. // RowLayout, ColumnLayout, GridLayout, StackLayout, Layout
import QtQuick.Dialogs 1.3 // MessageDialog (since Qt 5.2)
import QtGraphicalEffects 1.12 // LinearGradient
import Qt.labs.platform 1.1 // FileDialog (since Qt 5.8) // change to "import QtQuick.Dialogs" if using Qt 6.2

//import neroshop.FileDialog 1.0 // todo:check if custom module exists before impoorting it since its only available on linux
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
    minimumWidth: 750
    minimumHeight: 450    
    color: '#202020'
    /*style: ApplicationWindowStyle {
        background: BorderImage {
            source: "background.png"
            border { left: 20; top: 20; right: 20; bottom: 20 }
        }
    }*/    
    /*menuBar: MenuBar {
        // ...
    }

    header: ToolBar {
        // ...
    }

    footer: TabBar {
        // ...
    }   */

    /*StackView { // This would be good for subpages that can be stacked on top of each other
        id: home_stackview
        anchors.fill: parent // will fill entire Window area
        ////initialItem: page_loader
        //currentItem: 
    }*/
    ///////////////////////////
    /*NeroshopComponents.Hint {
        id: hint
        visible: false
    }*/
    NeroshopComponents.SearchBar {
        visible: (!page_loader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
    }
    NeroshopComponents.NavigationalBar {
        visible: (!page_loader.source.toString().match("qml/pages/MainPage.qml")) ? true : false;
    }
    ///////////////////////////
    // todo: create a top banner with a dot-styled pagination
    ///////////////////////////
    //Item {
    Loader {
        id: page_loader
        anchors.centerIn: parent // place at center of parent
        //anchors.fill: parent
        source: "qml/pages/MainPage.qml"
        //source: "qml/pages/HomePage.qml"
        //source: "qml/pages/CatalogPage.qml"
        //source: "qml/pages/ProductPage.qml"
        //source: "qml/pages/OrderCheckoutPage.qml"
        ////source: "qml/pages/Page.qml"

        onSourceChanged: {
            console.log(source);
            if (page_loader.status == Loader.Ready) console.log('Loaded') 
            else console.log('Not Loaded')
        }
    }
    //}
    // navigating between different pages: https://stackoverflow.com/a/15655043
    // Page does not render the title itself, but instead relies on the application to do so
    /*header: Label {
        text: main_page.title
        horizontalAlignment: Text.AlignHCenter
    }*/
}
// error: module "QtQuick.Layouts" version 1.15 is not installed (fix by changing the version specified in /usr/lib/x86_64-linux-gnu/qt5/qml/QtQuick/Layouts/plugins.qmltypes)
// error: module "Qt.labs.platform" is not installed (fix: sudo apt install qml-module-qt-labs-platform)
// error: gtk errors on "wallet_upload_button" clicked (partial_fix: https://bbs.archlinux.org/viewtopic.php?pid=1940197#p1940197)

// install: qtbase5-dev for "../cmake/Qt5/Qt5Config.cmake"
// error: /usr/lib/x86_64-linux-gnu/cmake/Qt5/Qt5Config.cmake will set Qt5_FOUND to FALSE so package "Qt5" is considered to be NOT FOUND due to Qml and Quick configuration files (/usr/lib/x86_64-linux-gnu/cmake/Qt5Qml/Qt5QmlConfig.cmake; /usr/lib/x86_64-linux-gnu/cmake/Qt5Quick/Qt5QuickConfig.cmake) not being found (fix: sudo apt install qtdeclarative5-dev)
// error: module "QtQuick" is not installed (fix: qml-module-qtquick-controls qml-module-qtquick-controls2). This will also install qml-module-qtgraphicaleffects, qml-module-qtquick-layouts, qml-module-qtquick-window2, and qml-module-qtquick2
// error: module "QtQuick.Shapes" is not installed (fix: qml-module-qtquick-shapes)

