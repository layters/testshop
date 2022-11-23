import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12
import QtGraphicalEffects 1.12

ListView {
    id: userReviews

    ScrollBar.vertical: ScrollBar {
           active: true
    }

    spacing: 20

    // FIXME: Implement me. This can be changed to a function.
    signal openUserReview(int reviewID)

    delegate: Rectangle {
        radius: 3
        width: userReviews.width
        height: col.implicitHeight + (col.anchors.margins * 2)

        // Try replacing with inline dropshadow
        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 1
        }

        MouseArea {
            anchors.fill: parent

            onClicked: userReviews.openUserReview(model.reviewID)
        }

        Column {
            id: col
            anchors.fill: parent

            anchors.margins: 10

            spacing: 5

            Item {
                height: productImage.height
                width: parent.width

                Label {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                    }

                    text: '"' + model.productName + '"'
                    font.pixelSize: 20
                }

                Image {
                    width: 70
                    height: 70

                    fillMode: Image.PreserveAspectCrop
                    mipmap: true

                    id: productImage
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: parent.right
                    }

                    source: model.productImage

                    layer.enabled: enabled
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            height: productImage.height
                            width: productImage.width
                            radius: 3
                        }
                    }
                }
            }

            Rating {
                value: model.rating

                Label {
                    leftPadding: 5
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.review
                    font.bold: true
                }
            }

            Label {
                wrapMode: Text.WordWrap
                width: parent.width
                text: model.description
                visible: model.description.length > 0
            }
        }
    }

}
