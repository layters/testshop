import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "." as NeroshopComponents
import FontAwesome 1.0

Row {//RowLayout {
    id: paginationBar
    spacing: 5
    property alias firstButton: backButton
    property alias secondButton: forwardButton
    property alias numberField: currentPageTextField
    property int page: 0
    property int totalPages: 0
    property real buttonWidth: 150
    property string buttonColor: "#50446f"//NeroshopComponents.Style.neroshopPurpleColor
    property real buttonRadius: 5
    property real radius: buttonRadius
    property bool showDirectionalIcons: false
    width: childrenRect.width; height: childrenRect.height
    function shouldBackButtonBeDisabled() {
        return page <= 1;
    }
    function shouldForwardButtonBeDisabled() {
        return page >= totalPages;
    }
    function update() {
        backButton.disabled = shouldBackButtonBeDisabled()
        forwardButton.disabled = shouldForwardButtonBeDisabled()
        currentPageTextField.text = qsTr(page.toString())
    }
    onPageChanged: {
        update()
    }
    onTotalPagesChanged: {
        update()
    }
    
    Button {
        id: backButton
        text: (!showDirectionalIcons) ? qsTr("Previous") : qsTr("%1  Previous").arg(FontAwesome.arrowAltCircleLeft)//.arg(FontAwesome.angleLeft)//qsTr("<")
        width: paginationBar.buttonWidth
        property bool disabled: shouldBackButtonBeDisabled()
        background: Rectangle {
            color: paginationBar.buttonColor
            radius: paginationBar.buttonRadius
            opacity: (backButton.disabled) ? 0.5 : 1.0
        }        
        contentItem: Text {
            text: backButton.text
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea {
            anchors.fill: parent
            onPressed: mouse.accepted = false
            cursorShape: !backButton.disabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
    }

    TextField {
        id: currentPageTextField
        width: 50
        //readOnly: true
        text: qsTr(page.toString())
        //horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter
        inputMethodHints: Qt.ImhDigitsOnly // for Android and iOS - typically used for input of languages such as Chinese or Japanese
        validator: RegExpValidator{ regExp: /[0-9]*/ }
        selectByMouse: true
        color: "#000000" // textColor
        background: Rectangle { 
            radius: paginationBar.radius
            //opacity: 0.0
        }        
    }

    Button {
        id: forwardButton
        text: (!showDirectionalIcons) ? qsTr("Next") : qsTr("Next  %1").arg(FontAwesome.arrowAltCircleRight)//.arg(FontAwesome.angleRight)//qsTr(">")
        width: paginationBar.buttonWidth
        property bool disabled: shouldForwardButtonBeDisabled()
        background: Rectangle {
            color: paginationBar.buttonColor
            radius: paginationBar.buttonRadius
            opacity: (forwardButton.disabled) ? 0.5 : 1.0
        }
        contentItem: Text {
            text: forwardButton.text
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            //font.family: "Font Awesome 6 Free"//FontAwesome.fontFamily//FontAwesome.fontFamilySolid
            //font.weight: Font.Bold
        }    
        MouseArea {
            anchors.fill: parent
            onPressed: mouse.accepted = false
            cursorShape: !forwardButton.disabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        }    
    }
}
