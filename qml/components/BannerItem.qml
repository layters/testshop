import QtQuick 2.12

Item {
    id: root

    default property alias content: container.data

    Item {
        id: container
        visible: false
        layer.enabled: true
        anchors.fill: root.visible ? parent : undefined
    }

    Rectangle {
        id: mask
        anchors.fill: container
        radius: 15
        layer.enabled: true
        visible: false
    }

    ShaderEffect {
        property var source: container
        property var maskSource: mask

        width: container.width
        height: container.height
        // For Qt5
        fragmentShader: "qrc:/assets/shaders/opacity_mask_qt5.fsh"
        // For Qt6
        // fragmentShader: "qrc:/assets/shaders/opacity_mask_qt6.frag.qsb"
    }
}
