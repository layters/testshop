import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
//import QtGraphicalEffects 1.12

// Wallet and Wallet Daemon settings page
import "../../components" as NeroshopComponents

Page {
    id: walletSettingsPage
    Rectangle {
        color: "#0a0a0a"
        visible: true
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: 20        
        ScrollBar.vertical.policy: ScrollBar.AsNeeded//ScrollBar.AlwaysOn
        clip: true    
    
        ColumnLayout {
            id: walletSettings
            anchors.fill: parent

            Image {
                source: "image://wallet_qr/%1".arg(Wallet.getPrimaryAddress())
                sourceSize {
                    width: 200
                    height: 200
                }
            }
        
            RowLayout {
                Rectangle {
                id: balanceDisplay
                radius: 5
                //color: "#0a0a0a"
        
                Image {
                    //id: lockedIcon
                }
                Label {
                    id: balanceLockedLabel
                }
                Label {
                    id: balanceLocked
                }
                }    
        /*Image {
            //id: unlockedIcon
        }       
        Label {
            id: balanceUnlockedLabel
        }
        Label {
            id: balanceUnlocked
        }*/         
        } // RowLayout

    } // root Layout

} // ScrollView
}
