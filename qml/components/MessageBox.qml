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
    property var onCloseCallback: null
    property alias title: messageBox.title
    property alias text: messageBox.text
    property alias model: buttonRepeater.model
    property alias buttonModel: buttonRepeater.model
    property var buttonAt: buttonRepeater.itemAt
    property int buttonCount: buttonRepeater.count
    property alias editModel: textEditRepeater.model
    property var editAt: textEditRepeater.itemAt
    property int editCount: textEditRepeater.count
    property alias textObject: messageBoxTextArea
    property alias textScroller: messageBoxTextScrollView
    property alias buttonRow: buttonsRow
    property alias editColumn: textEditsColumn
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
        	border.color: "#d9d9d9"
        	radius: 10//3
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
                    if(onCloseCallback != null) onCloseCallback()
                    messageBoxWindow.close()////messageBox.close()
                }
            }
        }
        // message box text
        ScrollView {
            id: messageBoxTextScrollView
            states: [
                State {
                    name: "hasEdit"
                    AnchorChanges {
                        target: messageBoxTextScrollView
                        anchors.top: titleBar.bottom
                    }          
                    PropertyChanges {
                        target: messageBoxTextScrollView.anchors
                        topMargin: 20
                    }
                },
                State {
                    name: "noEdit"
                    AnchorChanges {
                        target: messageBoxTextScrollView
                        anchors.verticalCenter: parent.verticalCenter
                    }          
                }                                                        
            ]
            state: (textEditRepeater.count > 0) ? "hasEdit" : "noEdit"
            width: parent.width
            height: messageBoxTextArea.height//120
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
        // edits column
        ColumnLayout {
            id: textEditsColumn
            states: [
                State {
                    name: "centered"
                    AnchorChanges {
                        target: textEditsColumn
                        anchors.horizontalCenter: parent.horizontalCenter
                    }          
                },
                State {
                    name: "filled"
                    PropertyChanges {
                        target: textEditsColumn
                        width: parent.width // = 470 = 500 - (messageBox.leftPadding + messageBox.rightPadding)
                    }          
                }                                                        
            ]            
            state: "centered"//"filled"
            anchors.top: messageBoxTextScrollView.bottom; anchors.topMargin: 10
            Repeater {
                id: textEditRepeater
                model: null
                delegate: TextField {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 330
                    property int editIndex: index
                    selectByMouse: true
                    background: Rectangle {
                        radius: 5//10
                    }
                }
            }
        }
        // buttons row    
        RowLayout {
            id: buttonsRow
            states: [
                State {
                    name: "left"
                    AnchorChanges {
                        target: buttonsRow
                        anchors.left: parent.left
                    }
                },
                State {
                    name: "right"
                    AnchorChanges {
                        target: buttonsRow
                        anchors.right: parent.right
                    }          
                },
                State {
                    name: "centered"
                    AnchorChanges {
                        target: buttonsRow
                        anchors.horizontalCenter: parent.horizontalCenter
                    }          
                },
                State {
                    name: "filled"
                    PropertyChanges {
                        target: buttonsRow
                        width: parent.width // = 470 = 500 - (messageBox.leftPadding + messageBox.rightPadding)
                    }          
                }                                                        
            ]
            state: "filled"//"centered"//"right"//"left"
            anchors.bottom: parent.bottom
            spacing: 10

            Repeater {
                id: buttonRepeater
                model: ["Close"]////null
                delegate: Button {
            	    Layout.fillWidth: true // If row width is set, button width will fill row width
            	    Layout.preferredWidth: contentItem.contentWidth + (contentItem.text.length * 10)//20//30
            	    Layout.preferredHeight: contentItem.contentHeight + 20
                    text: modelData
                    property int buttonIndex: index
                    property string color: "#b22222"
                    property string textColor: "#ffffff"
                    property var onClickedCallback: function() { messageBoxWindow.close() }////null
                    background: Rectangle {
                        radius: 18//5
                        color: parent.color
                    }
                    contentItem: Text {
                        text: parent.text
                        color: parent.textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onPressed: mouse.accepted = false
                        cursorShape: Qt.PointingHandCursor
                    }
                    onClicked: {
                        if(onClickedCallback != null) onClickedCallback()
                    }
                }
            }
        } // RowLayout     
	}
    ////DragHandler { target: messageBox }
}
