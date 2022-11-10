// This page displays the product page upon clicking on a product in the catalog view
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15


import "../components" as NeroshopComponents

import FontAwesome 1.0

Item {
    width: 700
    height: 500

    component Star: Label {
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

    component Rating: Row {

        property int value

        onValueChanged:{
            var stars = value
            var emptyStars = 5 - stars
            var count = 0

            while (stars > 0)
            {

                if (stars < 1)
                {
                    allStars[count].value = 1
                }
                else
                {
                    allStars[count].value = 2
                }

                count++;
                stars--;
            }

            while (emptyStars >= 1)
            {
                allStars[count].value = 0
                count++;
                emptyStars--;
            }
        }

        spacing: 5

        Star {
            id: star1
        }

        Star {
            id: star2
        }

        Star {
            id: star3
        }

        Star {
            id: star4
        }

        Star {
            id: star5
        }

        property var allStars: [star1, star2, star3, star4, star5]
    }

    SwipeView {
        id: sv
        anchors.fill: parent
        interactive: false
        orientation: Qt.Vertical

        currentIndex: 0

        onCurrentIndexChanged: {
            if (sv.currentIndex === 0)
            {
                productPageItem.opacity = 1
                reviewsPageItem.opacity = 0
            }
            else
            {
                productPageItem.opacity = 0
                reviewsPageItem.opacity = true
            }
        }

        Item {
            id: productPageItem

            Behavior on opacity {
                PropertyAnimation {}
            }

            Rectangle
            {
                color: "transparent"

                anchors {
                    margins: 10
                    fill: parent
                }

                id: productPage

                property string productID;
                property string sellerID;

                Component.onCompleted: {
                    // Debug value
                    loadProduct("p01",
                                "https://media.npr.org/assets/img/2022/07/31/19391873408_8af93aab12_o_sq-15394fcba9dec85dd46b7e9c55190ab2142ba9ae.jpg",
                                "Candy",
                                "$ 5.00",
                                4,
                                sampleReviewModel.count,
                                "Debayan's Candy Shop",
                                "6504",
                                "Best Candy you have <i>ever</i> tasted!<br/><b>Finger lickin' good!</b>")
                }

                // Sample List Model
                ListModel {
                    id: sampleReviewModel

                    ListElement {
                        name: "Jack"
                        rating: 4
                        review: "These candies are actually unique! Must try!"
                        description: "They really taste good. It melts in my mouth. Wrapping could have been better though."
                    }

                    ListElement {
                        name: "Jill"
                        rating: 1
                        review: "Ugliest and disgusting candies! Never try please"
                        description: "Pathetic packing, bland taste. Do yourself a favour and spend your money somewhere else!"
                    }

                    ListElement {
                        name: "Rock"
                        rating: 2
                        review: "Meh"
                        description: ""
                    }

                    ListElement {
                        name: "Stan"
                        rating: 5
                        review: ""
                        description: ""
                    }

                    ListElement {
                        name: "Rahul"
                        rating: 5
                        review: "Fantastic"
                        description: "I absolutely loved it!"
                    }

                    ListElement {
                        name: "Daisy"
                        rating: 4
                        review: "My mom loved it!"
                    }

                }

                function loadProduct(ID, productImageURL, name, price, reviewsStars, noOfReviews,
                                     sellerName, sellerID, description)
                {
                    productID = ID
                    image.source = productImageURL
                    nameLabel.text = name
                    priceLabel.text = price
                    reviews.value = reviewsStars
                    noOfReviewsLabel.text = noOfReviews + " reviews"
                    sellerNameLabel.text = sellerName
                    this.sellerID = sellerID
                    descriptionLabel.text = description
                }

                function openSellerPage()
                {
                    // FIXME: Implement
                    console.log("Open Seller page for ID", sellerID)
                }

                function buyNow()
                {
                    // FIXME: Implement
                    console.log("Bought product ID", productID)
                }

                function addToCart(quantity)
                {
                    // FIXME: Implement
                    console.log(quantity, "of product ID ",productID,"added to cart")
                }

                function chatWithSeller()
                {
                    // FIXME: Implement
                    console.log("Start Chat with Seller Dialog")
                }


                /*
                  Unclear if product page is supposed to be Dialog or not.
                  If Dialog, you can just call "destroy()" to close dialog.
                  */
                RoundButton {
                    id: closeButton

                    highlighted: true

                    Material.background: Material.Red
                    anchors {
                        top: parent.top
                        right: parent.right
                    }

                    // Did not use FontAwesome here because no entry for Close button was created
                    text: qsTr("X")
                    onClicked: {
                        // FIXME: Implement
                        console.log("Product page closed.");
                    }
                }

                Image {
                    id: image
                    height: 200
                    width: 200
                    anchors {
                        top: parent.top
                        left: parent.left
                    }

                    layer.enabled: true
                    layer.effect: ElevationEffect {
                        elevation: 6
                    }
                }

                Column {
                    anchors {
                        top: parent.top
                        left: image.right
                        right: parent.right
                        leftMargin: 10
                    }

                    spacing: 10

                    Label {
                        id: nameLabel
                        font.pixelSize: 40
                    }

                    Label {
                        id: priceLabel
                        font.pixelSize: 25
                    }

                    Rating {
                        id: reviews

                        Label {
                            id: noOfReviewsLabel

                            font.underline: true
                            color: "blue"

                            leftPadding: 5

                            MouseArea {
                                anchors.fill: parent

                                onClicked: sv.currentIndex = 1
                            }
                        }
                    }

                    Row {
                        Label {
                            text: qsTr("Sold By: ")
                        }

                        Label {
                            id: sellerNameLabel

                            font.underline: true
                            color: "blue"

                            MouseArea {
                                anchors.fill: parent

                                onClicked: productPage.openSellerPage()
                            }
                        }
                    }
                }

                Label {
                    id: descriptionHeading
                    anchors {
                        topMargin: 20
                        top: image.bottom
                        left: parent.left
                    }

                    font.bold: true

                    text: qsTr("Description And Specifications")

                    font.pixelSize: 18
                }

                ScrollView {
                    anchors {
                        top: descriptionHeading.bottom
                        left: parent.left
                        right: parent.right
                        topMargin: 10
                        bottom: pQuantity.top
                    }

                    Label {
                        id : descriptionLabel
                        wrapMode: Text.WordWrap
                    }
                }

                SpinBox
                {
                    id: pQuantity
                    anchors {
                        bottom : buttonRow.top
                        horizontalCenter: parent.horizontalCenter
                    }

                    from: 1
                }

                Row {
                    id: buttonRow
                    height: 50

                    spacing: 5

                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }

                    Button {
                        highlighted: true
                        Material.background: Material.Green
                        text: qsTr("Buy Now")
                        height: parent.height
                        width: (parent.width / 3) - parent.spacing

                        onClicked: productPage.buyNow()
                    }

                    Button {
                        highlighted: true
                        Material.background: Material.Orange
                        text: qsTr("Add to Cart")
                        height: parent.height
                        width: (parent.width / 3) - parent.spacing

                        onClicked: productPage.addToCart(pQuantity.value)
                    }

                    Button {
                        highlighted: true
                        Material.background: Material.Blue
                        text: qsTr("Chat")
                        height: parent.height
                        width: (parent.width / 3) - parent.spacing

                        onClicked: productPage.chatWithSeller()
                    }
                }
            }


        }

        Item {
            id: reviewsPageItem

            Behavior on opacity {
                PropertyAnimation {}
            }

            Rectangle {
                id: reviewHeader

                z: 1

                height: 50

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }

                layer.enabled: true
                layer.effect: ElevationEffect {
                    elevation: 3
                }

                RoundButton {
                    id: closeReviewsButton
                    flat: true
                    text: qsTr("←")
                    onClicked: sv.currentIndex = 0

                    anchors {
                        left: parent.left
                        leftMargin: 5
                        verticalCenter: parent.verticalCenter
                    }
                }

                Label {
                    text: qsTr("Reviews")

                    anchors {
                        left: closeReviewsButton.right
                        leftMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            ListView {

                anchors {
                    margins: 10
                    left: parent.left
                    right: parent.right
                    top: reviewHeader.bottom
                    bottom: parent.bottom
                }

                ScrollBar.vertical: ScrollBar {
                       active: true
                }

                spacing: 20

                // FIXME: Replace model with proper QAbstractItemModel
                model: sampleReviewModel

                delegate: Rectangle {

                    radius: 3
                    width: parent.width
                    height: col.implicitHeight + (col.anchors.margins * 2)


                    layer.enabled: true
                    layer.effect: ElevationEffect {
                        elevation: 1
                    }

                    Column {
                        id: col
                        anchors.fill: parent

                        anchors.margins: 10

                        spacing: 5

                        Label {
                            text: model.name
                            font.pixelSize: 20
                        }

                        Rating {
                            value: model.rating

                            Label {
                                leftPadding: 5
                                text: model.review
                                font.bold: true
                            }
                        }

                        Label {
                            wrapMode: Text.WordWrap
                            width: parent.width
                            text: model.description
                        }
                    }
                }
            }

        }
    }
}
