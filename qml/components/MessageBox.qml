import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
//import QtGraphicalEffects 1.12

import FontAwesome 1.0
import "." as NeroshopComponents

Window {
    id: messageBoxWindow
    function open() {
        messageBoxWindow.show()
        messageBox.open()
    }    
    width: 500; height: 250    
    property alias title: messageBox.title
    property alias text: messageBox.text
    property alias closeCancelButton: closeButton // close or cancel
    property alias acceptButton: okButton
    property Button button2: null
    property Button button3: null
    property alias textObject: messageBoxTextArea
    property alias buttonRow: messageBoxButtonRow
    visible: messageBox.visible
    modality: Qt.ApplicationModal//Qt.WindowModal // A modal window prevents other windows from receiving input events (ESC closes window when modality is set)
    flags: Qt.FramelessWindowHint | Qt.Dialog
    color: "transparent"////"blue"
	
	Popup {
    	id: messageBox
    	visible: false
    	anchors.centerIn: parent
    	property string title: "message"
    	property string text: ""
    	leftPadding: 15; rightPadding: leftPadding
    	//bottomPadding: 0
    	background: Rectangle {
        	implicitWidth: messageBoxWindow.width
        	implicitHeight: messageBoxWindow.height
        	color: "#a0a0a0"////NeroshopComponents.Style.getColorsFromTheme()[2]
        	border.color: "white"
        	radius: 3
    	} // background
        // title bar
        Rectangle {
            id: titleBar
            color: "#323232"
            height: 40
            width: parent.width
            radius: messageBox.background.radius
            // title text
            Text {
                id: titleText
                text: messageBox.title
                anchors.centerIn: parent////anchors.verticalCenter: titleBar.verticalCenter; anchors.horizontalCenter: titleBar.horizontalCenter
                ////anchors.left: titleBar.left; anchors.leftMargin: 10
                color: "#ffffff"
                font.bold: true
            }
            // title bar close button
            Button {
                id: titleBarCloseButton
                width: 20//25
                height: this.width

                anchors.verticalCenter: titleBar.verticalCenter
                anchors.right: titleBar.right
                anchors.rightMargin: 10
                text: qsTr(FontAwesome.xmark)
                contentItem: Text {  
                    text: titleBarCloseButton.text
                    color: "#ffffff"
                    font.bold: true
                    font.family: FontAwesome.fontFamily
                    font.pointSize: 8//11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? "#ff4d4d" : "#ff4444"
                    radius: 100
                }
                onClicked: {
                    messageBoxWindow.close()////messageBox.close()
                }
            }
        }
        // message box text
        ScrollView {
            anchors.verticalCenter: parent.verticalCenter////anchors.top: titleBar.bottom; anchors.topMargin: 20
            width: parent.width
            height: 120
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            
            TextArea {
                id: messageBoxTextArea
                text: messageBox.text
                readOnly: true
                wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width//Text.WordWrap does not move text to the newline but instead it only shows the scrollbar
                selectByMouse: true
                color: "#000000"
                verticalAlignment: TextEdit.AlignVCenter // align the text within the center of TextArea item's height
                horizontalAlignment: TextEdit.AlignHCenter
            }    
        }
        // buttons row    
        RowLayout {
            id: messageBoxButtonRow
            ////layoutDirection: Qt.RightToLeft
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter////anchors.right: parent.right; anchors.rightMargin: 20
            anchors.bottom: parent.bottom
            // close button
        	Button {
            	id: closeButton
            	text: qsTr("Close")
            	Layout.preferredWidth: contentItem.contentWidth + (contentItem.text.length * 10)//20
            	Layout.preferredHeight: contentItem.contentHeight + 20
            	background: Rectangle {
                	color: "firebrick"
                	radius: 5      
            	}
            	contentItem: Text {
                	text: parent.text
                	color: "#ffffff"
                	horizontalAlignment: Text.AlignHCenter
                	verticalAlignment: Text.AlignVCenter                
            	}
            	onClicked: {
                	messageBoxWindow.close()
            	}            
        	}      
            // OK button
        	Button {
            	id: okButton
            	text: qsTr("OK")
            	Layout.preferredWidth: contentItem.contentWidth + (contentItem.text.length * 10)//20
            	Layout.preferredHeight: contentItem.contentHeight + 20       
            	visible: false     
            	background: Rectangle {
                	color: "#4169e1"//"#4682b4"
                	radius: 5
            	}            
            	contentItem: Text {
                	text: parent.text
                	color: "#ffffff"
                	horizontalAlignment: Text.AlignHCenter
                	verticalAlignment: Text.AlignVCenter                
            	}         
            	////onClicked:   
        	}               
        } // RowLayout     
	}
    ////DragHandler { target: messageBox }
}
