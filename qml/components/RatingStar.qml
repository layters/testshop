import QtQuick 2.12
import QtQuick.Controls 2.12

Label {
    color: "red"
    width: 20
    height: 20

    property int value

    Component.onCompleted: configure()

    function configure()
    {
        if (value === 0) // Empty
        {
            text = FontAwesome.star
        }
        else if (value === 1) // Half
        {
            text = "<b>"+FontAwesome.starHalf+"</b>"
        }
        else if (value === 2) // Full
        {
            text = "<b>"+FontAwesome.star+"</b>"
        }
    }

    onValueChanged: configure()
}