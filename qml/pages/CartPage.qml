import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../components" as NeroshopComponents

Page {
	id: cartPage
	background: Rectangle {
        color: "transparent"
    }
    Button {
        text: "checkout"
        width: parent.width
        height: 100
        onClicked: pageStack.pushPage("qrc:/qml/pages/OrderCheckoutPage.qml", StackView.Immediate)
    }
}

