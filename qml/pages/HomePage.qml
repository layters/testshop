// consists of the navigational bar with menus and search bar
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "../components" as NeroshopComponents // Tooltip
// number to string: my_number.toString()
// string to number: Number(my_string)
Page {
    id: home_page
    background: Rectangle {
        //visible: true
        color:"transparent" // fixes white edges on borders when grid box radius is set
    }
}
