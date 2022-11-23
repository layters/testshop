import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import QtQuick.Dialogs 1.3
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

import Icons 1.0

import "../../components" as NeroshopComponents

Item {
    id: settingsAccount

    implicitWidth: 500
    implicitHeight: 700

    //Sample data. This must be removed later.
    Component.onCompleted : {
        loadUserDetails(20,
                        "Jack",
                        "https://www.wipo.int/export/sites/www/wipo_magazine/images/en/2018/2018_01_art_7_1_400.jpg",
                        sampleUserReviewsModel,
                        sampleUserFavouritesModel)

    }

    // Sample user reviews. This must be removed later.
    ListModel {
        id: sampleUserReviewsModel

        ListElement {
            reviewID: 0
            productName: "Hotwheeliez"
            productImage: "https://www.scalemodelcart.com/usrfile/40002-18_MCG_18267_Chevrolet_Caprice_Classic_a.jpg"
            rating: 3
            review: "Okish"
            description: "Wheels are good. Body paint average."
        }

        ListElement {
            reviewID: 1
            productName: "Bottle (Pack of 3)"
            productImage: "https://upload.wikimedia.org/wikipedia/commons/0/07/Multi-use_water_bottle.JPG"
            rating: 5
            review: "Perfect"
            description: "Worth the price"
        }

        ListElement {
            reviewID: 2
            productName: "Adult Mouse"
            productImage: "https://upload.wikimedia.org/wikipedia/commons/8/8f/Mouse_white_background.jpg"
            rating: 1
            review: "Black Death"
            description: "The one I had received had the plague!"
        }
    }

    // Sample user favourites. This must be removed later.
    ListModel {
        id: sampleUserFavouritesModel

        ListElement {
            productID: 0
            name: "Candy"
            image: "https://media.npr.org/assets/img/2022/07/31/19391873408_8af93aab12_o_sq-15394fcba9dec85dd46b7e9c55190ab2142ba9ae.jpg"
            price: "$ 5.00"
        }

        ListElement {
            productID: 1
            name: "Bottle"
            image: "https://upload.wikimedia.org/wikipedia/commons/0/07/Multi-use_water_bottle.JPG"
            price: "$ 3.00"
        }

        ListElement {
            productID: 2
            name: "Hotwheeliez"
            image: "https://www.scalemodelcart.com/usrfile/40002-18_MCG_18267_Chevrolet_Caprice_Classic_a.jpg"
            price: "$ 10.00"
        }
    }

    // Sample new user photo controllers. This must be removed later.
    onSetNewUserPhoto: (newFilePath) => {
                           loadUserDetails(20,
                                           "Jack",
                                           newFilePath,
                                           sampleUserReviewsModel,
                                           sampleUserFavouritesModel)
                       }

    onRemoveUserPhoto: {
        loadUserDetails(20,
                        "Jack",
                        "",
                        sampleUserReviewsModel,
                        sampleUserFavouritesModel)
    }

    signal setNewUserPhoto(string newFilePath)
    signal removeUserPhoto()

    property bool isUserPhotoPresent

    function loadUserDetails(id, name, photoSource, reviewsModel, favouritesModel)
    {
        userFullName.text = name
        userPhoto.source = (photoSource.length > 0) ? photoSource : Icons.defaultUserPhoto
        isUserPhotoPresent = photoSource.length > 0

        reviews.model = reviewsModel
        favourites.model = favouritesModel
    }


    SwipeView {
        id: sv

        anchors {
            fill: parent
        }

        interactive: false

        function showFavourites()
        {
            currentIndex = 1
            favourites.visible = true
            reviews.visible = false
        }

        function showReviews()
        {
            currentIndex = 1
            favourites.visible = false
            reviews.visible = true
        }

        Item {

            Item {
                id: accountSettingsPage
                anchors.fill: parent

                anchors.margins: 10

                Image {
                    id: userPhoto
                    anchors {
                        top: parent.top
                        horizontalCenter: parent.horizontalCenter
                    }

                    fillMode: Image.PreserveAspectCrop
                    mipmap: true
                    width: 150
                    height: 150

                    layer.enabled: enabled
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            height: userPhoto.height
                            width: userPhoto.width
                            radius: userPhoto.width / 2
                        }
                    }

                }

                RoundButton {
                    anchors {
                        right: userPhoto.right
                        bottom: userPhoto.bottom
                    }

                    icon.source: Icons.pencil

                    height: 60
                    width: 60

                    highlighted: true
                    onClicked: userPhotoMenu.visible ? userPhotoMenu.dismiss() : userPhotoMenu.open()

                    Menu {
                        id: userPhotoMenu

                        MenuItem {
                            text: qsTr("Select New Photo")
                            onTriggered: photoSelectFileDialog.open()
                        }

                        MenuItem {
                            text: qsTr("Remove photo")
                            onTriggered: removeUserPhoto()
                            height: isUserPhotoPresent ? implicitHeight : 0
                        }
                    }
                }

                FileDialog {
                    id: photoSelectFileDialog
                    nameFilters: ["Image (*.jpg *.jpeg *.png)"]
                    onAccepted: setNewUserPhoto(fileUrls[0])
                }

                Label {
                    id: userFullName
                    anchors {
                        top: userPhoto.bottom
                        topMargin: 15
                        horizontalCenter: parent.horizontalCenter
                    }

                    font.pixelSize: 25
                    font.bold: true
                }

                Rectangle {
                    id: bar1
                    color: "#e0e0e0"
                    height: 1

                    anchors {
                        top: userFullName.bottom
                        topMargin: 20
                        left: parent.left
                        right: parent.right
                    }
                }

                Label {
                    id: accountLabel

                    anchors {
                        top: bar1.bottom
                        topMargin: 10
                        left: parent.left
                    }

                    text: qsTr("Account")
                    font.pixelSize: 20
                }

                Button {
                    id: myFavouritesBtn
                    anchors {
                        top: accountLabel.bottom
                        topMargin: 5
                        left: parent.left
                    }

                    width: 130
                    height: 130

                    Item {
                        implicitHeight: favIcon.height + favLabel.anchors.topMargin + favLabel.implicitHeight
                        implicitWidth: Math.max(favIcon.width, favLabel.implicitWidth)

                        anchors {
                            centerIn: parent
                        }

                        Image {
                            id: favIcon
                            anchors {
                                top: parent.top
                                horizontalCenter: parent.horizontalCenter
                            }

                            source: Icons.heartFilled

                            fillMode: Image.PreserveAspectCrop
                            mipmap:true

                            width: 35
                            height: 35
                        }

                        ColorOverlay {
                            anchors.fill: favIcon
                            source: favIcon
                            color: "white"
                        }

                        Label {
                            id: favLabel
                            anchors {
                                top: favIcon.bottom
                                topMargin: 15
                                horizontalCenter: parent.horizontalCenter
                            }

                            text: qsTr("My Favourites")
                            font.pixelSize: 14
                            color: "white"
                        }
                    }



                    highlighted: true

                    enabled: favourites.model.count > 0

                    Material.background: Material.Pink

                    onClicked: sv.showFavourites()
                }

                Button {
                    id: myReviewsBtn
                    anchors {
                        top: accountLabel.bottom
                        topMargin: 5
                        left: myFavouritesBtn.right
                        leftMargin: 5
                    }

                    width: 130
                    height: 130

                    Item {
                        implicitHeight: revIcon.height + revLabel.anchors.topMargin + revLabel.implicitHeight
                        implicitWidth: Math.max(revIcon.width, revLabel.implicitWidth)

                        anchors {
                            centerIn: parent
                        }

                        Image {
                            id: revIcon
                            anchors {
                                top: parent.top
                                horizontalCenter: parent.horizontalCenter
                            }

                            source: Icons.reviews

                            fillMode: Image.PreserveAspectCrop
                            mipmap:true

                            width: 35
                            height: 35
                        }

                        ColorOverlay {
                            anchors.fill: revIcon
                            source: revIcon
                            color: "white"
                        }

                        Label {
                            id: revLabel
                            anchors {
                                top: revIcon.bottom
                                topMargin: 15
                                horizontalCenter: parent.horizontalCenter
                            }

                            text: qsTr("My Reviews")
                            font.pixelSize: 14
                            color: "white"
                        }
                    }



                    highlighted: true

                    enabled: reviews.model.count > 0

                    Material.background: Material.Green

                    onClicked: sv.showReviews()
                }

                Rectangle {
                    id: bar2
                    color: "#e0e0e0"
                    height: 1

                    anchors {
                        top: myFavouritesBtn.bottom
                        topMargin: 10
                        left: parent.left
                        right: parent.right
                    }
                }

                Label {
                    id: dangerLabel

                    anchors {
                        top: bar2.bottom
                        topMargin: 10
                        left: parent.left
                    }

                    text: qsTr("Danger")
                    font.pixelSize: 20
                }

                Button {
                    anchors {
                        top: dangerLabel.bottom
                        topMargin: 5
                        left: parent.left
                    }

                    icon.source: Icons.deleteForever
                    text: qsTr("Delete Account")
                    Material.background: Material.Red
                    highlighted: true

                    onClicked: {
                        // FIXME: Implement me
                        console.log("DELETE ACCOUNT!")
                    }
                }

                Button {
                    anchors {
                        bottom: parent.bottom
                        horizontalCenter: parent.horizontalCenter
                    }

                    icon.source: Icons.logout

                    text: qsTr("Logout")
                    highlighted: true
                    Material.background: Material.Red

                    onClicked: {
                        // FIXME: Implement me
                        console.log("LOGOUT!")
                    }
                }

            }

        }

        Item {
            id: userReviewsFavouritesPage

            Rectangle {
                id: userReviewsFavouritesPageHeader
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

                    icon.source: Icons.arrowBack

                    onClicked: sv.currentIndex = 0

                    anchors {
                        left: parent.left
                        leftMargin: 5
                        verticalCenter: parent.verticalCenter
                    }
                }

                Label {
                    text : reviews.visible ? qsTr("My Reviews") : qsTr("My Favourites")

                    anchors {
                        left: closeReviewsButton.right
                        leftMargin: 10
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            NeroshopComponents.UserReviews {
                id: reviews

                anchors {
                    margins: 10
                    left: parent.left
                    right: parent.right
                    top: userReviewsFavouritesPageHeader.bottom
                    bottom: parent.bottom
                }

                onOpenUserReview: (reviewID) => {
                                      console.log("Open review with ID: ", reviewID)
                                            }
            }

            NeroshopComponents.UserFavourites {
                id: favourites

                anchors {
                    margins: 10
                    left: parent.left
                    right: parent.right
                    top: userReviewsFavouritesPageHeader.bottom
                    bottom: parent.bottom
                }

                onRemoveFromUserFavourites: (productID) => {
                                                console.log("Remove favourite with product ID: ", productID)

                                                var favModel = favourites.model

                                                for (var i=0; i<favModel.count; i++)
                                                {
                                                    if (favModel.get(i).productID === productID){
                                                        favModel.remove(i);
                                                        break;
                                                    }
                                                }

                                                if (favModel.count === 0)
                                                {
                                                    sv.currentIndex = 0
                                                }
                                            }

                onOpenProduct: (productID) => {
                                                console.log("Remove product ID: ", productID)
                                            }
            }
        }

        Item {
            id: userFavouritesPage

            anchors.margins: 10
        }
    }

}
