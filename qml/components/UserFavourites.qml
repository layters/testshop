import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12
import QtGraphicalEffects 1.12

import Icons 1.0

ScrollView {
    id: userFavourites

    property alias model: rpt.model

    contentWidth: width

    // FIXME: Implement me. This can be changed to a function.
    signal removeFromUserFavourites(int productID)
    signal openProduct(int productID)

    Flow {
        id: flow
        width: parent.width
            spacing: 10

            Repeater {
                id: rpt

                delegate: Rectangle {
                    radius: 3
                    width: innerBox.implicitWidth + (innerBox.anchors.margins * 2)
                    height: innerBox.implicitHeight + (innerBox.anchors.margins * 2)

                    // Try replacing with inline dropshadow
                    layer.enabled: true
                    layer.effect: ElevationEffect {
                        elevation: 1
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: userFavourites.openProduct(model.productID)
                    }

                    Item {
                        id: innerBox
                        anchors.margins: 10
                        anchors.centerIn: parent

                        implicitHeight: productImage.height + productDetails.implicitHeight + 10
                        implicitWidth: productDetails.implicitWidth

                        Image {
                            id: productImage

                            anchors {
                                top: parent.top
                                left: parent.left
                            }

                            height: productDetails.implicitWidth
                            width: productDetails.implicitWidth

                            fillMode: Image.PreserveAspectCrop
                            mipmap: true

                            source: model.image

                            layer.enabled: enabled
                            layer.effect: OpacityMask {
                                maskSource: Rectangle {
                                    height: productImage.height
                                    width: productImage.width
                                    radius: 3
                                }
                            }
                        }

                        Item {
                            id: productDetails

                            anchors {
                                top: productImage.bottom
                                topMargin: 10
                                left: parent.left
                            }


                            implicitHeight: Math.max(productName.implicitHeight, productPrice.implicitHeight, favouriteButton.implicitHeight)
                            implicitWidth: Math.max(200, (productName.implicitWidth +
                                                          productPrice.anchors.leftMargin + productPrice.implicitWidth +
                                                          10 +
                                                          favouriteButton.implicitWidth))

                            Label {
                                id: productName

                                anchors {
                                    left: parent.left
                                    verticalCenter: parent.verticalCenter
                                }

                                text: model.name
                                font.bold: true
                            }

                            Label {
                                id: productPrice

                                anchors {
                                    left: productName.right
                                    leftMargin: 10
                                    verticalCenter: parent.verticalCenter
                                }

                                text: model.price
                            }

                            RoundButton {
                                id: favouriteButton

                                anchors {
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                }

                                icon.source: Icons.heartFilled

                                flat: true
                                Material.foreground: Material.Pink

                                onClicked: userFavourites.removeFromUserFavourites(model.productID)
                            }
                        }

                    }


                }

            }

    }

}

