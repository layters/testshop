import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

import Icons 1.0

Item {
    height: 30
    width: 30

    property int value

    Component.onCompleted: configure()

    function configure()
    {
        if (value === 0) // Empty
        {
            image.source = Icons.star
        }
        else if (value === 1) // Half
        {
            image.source = Icons.starHalf
        }
        else if (value === 2) // Full
        {
            image.source = Icons.starFilled
        }
    }

    onValueChanged: configure()

    Image {
        id: image

        width: 30
        height: 30

        fillMode: Image.PreserveAspectCrop
        mipmap: true
    }

    ColorOverlay {
        anchors.fill: image
        source: image
        color: "red"
    }


}

