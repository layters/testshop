// consists of the navigational bar with menus and search bar
import QtQuick 2.12//2.7 //(QtQuick 2.7 is lowest version for Qt 5.7)
import QtQuick.Controls 2.12 // StackView
import QtQuick.Layouts 1.12 // GridLayout
import QtQuick.Shapes 1.3 // (since Qt 5.10) // Shape
import QtGraphicalEffects 1.12//Qt5Compat.GraphicalEffects 1.15//= Qt6// ColorOverlay

import "../components" as NeroshopComponents // Tooltip

Page {
    id: homePage
    background: Rectangle {
        color: "transparent" // Make transparent to blend in with theme
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: 20        
        ScrollBar.vertical.policy: ScrollBar.AsNeeded//ScrollBar.AlwaysOn
        clip: true
        
        GridLayout {
            anchors.fill: parent
            // Banner
            NeroshopComponents.Banner {
                id: homeBanner
                Layout.preferredWidth: scrollView.width////parent.width // will not fill page if you use parent.width. Use scrollView.width instead
                Layout.row: 0 // sets the row position

                NeroshopComponents.BannerItem {
                    Rectangle {
                        id: firstPage
                        anchors.fill: parent
                        color: "blue"

                        //Image {
                        //    anchors.fill: parent
                        //    source: "https://revuo-xmr.com/img/img-issue151.png"//"file:///" + neroshopAppDirPath + "/img-issue150.png"//"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAbEAAAB0CAMAAAA8XPwwAAAAyVBMVEX///9NTU3yaCJKSkpGRkbT09M+Pj7Y2NhpaWne3t7z8/NZWVldXV35+fnPz8+UlJTyYhD0fkv+6+H3q5BlZWVCQkK+vr6Kior5wa9SUlLt7e3yZRvo6OiCgoI3Nzfi4uKysrKdnZ15eXnDw8PxXQCrq6uFhYWtra1wcHCXl5f0fT+ioqL6zrv+9/P+8er95dr708H729H1i1rzczH1hlX2nHP5u6H2lGv2oYD6wKb3pX3ybCjzdDv5uJj3qYn3oHT0g0kuLi73p4s1gV76AAASb0lEQVR4nO2deXvauhLGITI7BprYpsYQjAkJCSF71+T09J7v/6GubazRNhJL7NKn9fvHXeoFST9pNDOSnEql1F+i9Xp97CKU2kl3948Pt7E+n519jv/r4eH+tET32+o+5vTl6e15cHFxMYgV/+fr88vT17NvD3fHLlspRQ9nn16eg0EQBCeCgmAwOLl5+vrt9NglLMXp/vvb84nMSsAWvL69/FPax99D63+eLwZ6Wtxgu3h6LKEdW+vH78EOuCi1i5uP5ZR2TK0fP8Wz1D4KBk/ljHY8Pf543o8XZVaOs6Po7scHg7NhYnby5eHYhf8b9XBzchCvlNnrj9IF+cVa/3txMK9Eg5PHY1fh79Ljy/4TmDTMLsrw7Ndp/e3tvcCSYfbl/tgV+Vu0Pnt9l0UEZC+lA/JLdPfpMBdRVXBze+zK/A1av+TEK0H2+u3Y1fnzdfohhymMIbv4cewK/em6f8pvhG2YlcgK1emXPEdYSuy5NIwF6u5T3sBiZG+l+1GcPuVsEjfIbkonvyj9yH+Epcjeylx+Mbq9KARYMsqOXbU/U6fPRdjEVIPvx67cn6j1l8KAxaOsnMry1+fieMXEPpRbCfLW44cCh1iscokzZ91xjv1rThNa8PrK/e8yKstXtwxS8HL7Mw9kwcnHMxYuBE+lXcxTdxyj4MPdfQ75+yRvzxE7GXw8diX/KH3jQrHES3h/Qjg4+VYRiAU/y0GWn9b8zJX6dffv3egRJAlgntjJoEwJ5yd+iGWe+OnNu5ClwERiwU3pLuYmwTnMYqf7dyALgs2kJRA7GXw+aiX/JH0TJi0a7Z4e7DHGXuLmxWcS9CPW8c/SE0qscnqg+xE8U7dQIla6iznp9llscMgoHeYxMmAyseBJXwi7Ucs0xq+36PWpuTajVvamRmtk625q05fVRthlKMsU3tDu1cxi727gN7Sm47a56Gk9x1D+KVq2RD9ELlwO8P7pkLMtLLshE3vT7zltWfWNziP0+rgL17UcKpXp/GrodDY3+s5kNW9pfs2nbxtil7vn9Cq0ce9/dbPg4VoXve52nagf9rQUNuWaryKnurnfb06u5mj3lY0fn7U9/bkvMup0pJLnsZN/tGWtudVMFtoTe1Z2mXS0xBqrpe9ahNA7iesvZzXszlaX3uT2kMvN7KrVZ8TqVaMIKwV9Wr6DEMvtTjx9h+vNllWXK75l+dEK6XOSURTz7Ps6+UHA5w9lYsEXbWk9IOZ6yGW7T4lV65oqj68dorQVsbp9xIwyYqSLdJBCiGUY/L5mmNWGXaT8xOkr9VWaVVgZ2W+UBSe35lfrzKI9BCJkiV33oTJ1bFRUKmFHre+mzt1QqTMjFlNR31UcsaQPRagRmVW15Z+Ld57KK5nSWtY+HmPwLGboFasY6EKy9pIV10I64Zw1mLVCnh9FdW07EXcoTwccMeKrY7pIYvFbmyqyluNq7yfWTHjg8VluVWn1UUG6MzCV2OA/pLUTtTusmiRUr0cc0Ev1csu3sLpSuUvJMnLEqtZSmd6LJVZ15WFtex1j+S2hy90q40BeL77fEZm6BqaOMd2ay/icqzziLXIVIh3l6tzZ2q1FZDyxanUlW82CiVVd0Z2wF/628nPI1v9sJRYbxl3mMtHpSKUSu9Ec3eQbhHQVX2HO24y68rAILPaxXNeyhH+zlg3+CYEY6cozo5EYQWXBwxwx/gahNKKZCH2k/NITE5gq7pQBhOzJuNvB/UCAqcROkJtSrXir4F/Jlyf85bpkxVp8pyau2+wvQm81bJ7z0MiEf0ocYySSZk4jsa6DCiHm89cJVxyxT859ofxkebnwwv7Er/OVtsDwnN7sQGwHZBgwhJguURXxLWhdSmaqLTSwK/pOIw4ncbvhyE6ftu3xrMMxs6656VskpjgzBmKk04tfjUglZg2FO0czNtnyfXJc5aZw1/Gg/K0hbyigjPfKtlJ035M6FKWHnrH9bQixr/iSS0fo844U9oZ888rtu+LcyKbo99kLjowbsgsSsWpd/MEtxMziiYkaTeiv8peaLK6pNsXO2B6y4Qc+7eNuxLZ4jJrjEIhVxF0Pm3M8qqq3OBTt/IS/1mIVJpdKXNCIYAYkXTaVycSIIzxVELHKlE64ZAJjMmRMOjPF7Z8zi0992s9qo+J7C03IFLc+E0LsJxpDN0RPzBoKRZ+K3pfYvBDIETJD3jxmobnLsgcysaq74B8qiljlmhKL6Kw6ZuXvqJF+pVLjAtVNGdXTEbrdoPqzZYFuy69K7OQZJeaJ8SNxBM8u9MWrfGKpxlgrTnqq9oSNMnCqFWLE50EURixUiC2gBCiw+HXgCJNNVPnvzsQqa437oQWGERug7n1fCiAtfj6yr8WrpMPNOuB2kKEmXzdd0ltYQkohJvqLhRHzZGJTcLnIteZ9U+iTaaus1RyUfsc1jkwPDCN2gRJbSg3INVXcvPJVLq80pS6L4q0w9WD6hkhOJVYlnF0sjFhfJubBzzvanH5IkaW9aq0496Y98thpiuBNfwpiZ2JKzO9zWQFPaV02YYFRqS6Q125kQ7AHywIYMYehKIrYmFo4K/M82mA+NPntVBGUv7UvscrdJ+Vu0wlMjBh2ewsmKli2YsOI1QoMCGsK6i+TJr50nYq1Ic01ADGrDz9tMU/TSEw7lpVfk4lNL6EGmX2G4J80TW+EVgl3ICaN1VPp4K0CTLgfm8ew7P2ctpoP7i9baWYucbMqXwQvkiBrJkxgjLrZPwAxt3fNllJDer8p59FZ1HqYWPtCmSLxzpBFXn72UzWw19KKiiCI45KKbyW2kOZzMZQOXuUhE/J9fVdi1LaR5gyaD94zt2hzr+A2ajPnYN1MNQZnk3Sy7BAj5rUhdCAd6qEas1R+BxX8GJelEm7wXS5LRaexrLsQ37ibIKS3dbYTsx15yXTNpYWDQJ6V5lU+Lb0rMRohk0mLDijIULRpQGX1e7TCMOUACt+46QU8ZD8DyxFj+xHiGDW735y7x1PB7MfwTLCQhsvsJcywXESNqUdNd32HMeYIblsi5jEiwPzzQ4hRFzeeoKifD2FymzMc0M3pLAeDs4u8lQnWS+mDPDGbpbloNyl4tYW42ShgvRGPJanAW643diBG6nKgY2eGMXiTgXl1Ut9CDPMVx1DHFXN3z7M6XFGLsBxXwF+gmcUZ9FFThblEc+Zl8sQqo4iNso3VLJYYgWizfZn9Mp/zRARRm9uLB8xWYlVXRrbxGIMP8hyWLO0fQqzXZUYLuhOtBP3/1syGcNmi0eauxC5NxLjljsynKZQY8SE4YcSM0zAbiwkxNcJSiMXIJMOYIFOBeYmtPoSY18kqk0RhLMhMr41oSyVTEDP72cy9K7G+kZh9DW28WQYpkpjlsGwUEKtr9lXS+/qMmL0tS5XO2a7sftx9GjwrJjFNP2wj9orkFVcUUuJwMWcipbIAo9hiS9GkmXl1uxKbGInx6dj0zcURs6whF8/tOsZGl4xYRdk0gBFTk3Z333FgW4lhmWDwNhJfjSXqwwrfeElbj7JFGVjnX4kDUiuYx7IJUCLGAsI4sGsXSWzS483VIfNY5eOW9bHMLyayYZQXJr1sLthCDFsfg3JvnF4IF5NxM6VV7aRNS5uNzt0s0DLWeEQbEfMVU12yvMrcTIzUz1HBjzFihG7gZkZXqjn4ijOjrwjRSdK629agIT5SVwt5eS73ThBC7Iv6jSqo4qanUW8xXXKBQbRZzoM7syzinCaCfUOSindtsrGpEKuwzdNxlJ1PliqOLhuJWh54NtJ+Z5s5wsZ4sgcRTgz2/nU3YmZkHtiMLcQGZ+quAWjPejo7tWn9klQORGqbLNQ1RNObGtageNjWb9AVDMWsDioxlm62JnlngtlykbxnvEdTZMTU42yYzJMEv3quT0PMhIxbkNxGDNmZI4dgEdTYrnF+fyJI12SrFXbEm1Cd2jQqgHyrSoxbonPny3yJcR2LiHtUGh3BvGg0WrIWieej7+b9itxWQCUuo+L3220hhm3gAdOQWXm6Y5s0W1dS36S7C4iTJQhZjrehvpgKbCcsaSLEmMdDHCdnYpUZ80WFvZhj6HGmpM0cVlvS0n6TJzItsbgIKLKQ3whlJha8qK4irKZQh8+G3r64lCo/or9kZYDmNONG1CMgVCPY6gFbphBise9Ef5YZyLyIsaV/cR3PXlHrhJ7oyRqIOZ9p4z7sOI+lxcWQecJypJnY4JM6jUE/gxEQQW6RhtZ0vW8EU14WwNj0H7ADD7SAjAD9J4yY3VcOK+S3otlng0wwBj1YQ+9q5xxYkM02BivfxzEQi59RZshQ3C69ZYwheWDofxBGqrEPbWqWrrnKhhR0Umk3D1MN3EC2YQojhgRS+RGDnlV1BUceIpuqpTt7yo71ZIVdfzWeH5POILgyslBqXSMxdIdBDR6lJr7dkZoORp9NkxzWJGtLG3hYOLIaW7pmi1AoMXnPVq67BkLoWV0hPPCgrkReJMlexyU9swb6mPx9Z15vBmIyMhmYSOw/6c0XL8gXg6EPwRqXvHmq6kIlPSWACdkasoM0Zo3ZbIttncaJ2ZdKT8lvnweLyQQybUigxXOOeprU5o7t0IXryulHSbf8XKOc83HZKYuKHSrnCgVij/KrkSFmLyBHxZpH2u7WBYPRgCu0FG22VkKqV5IFGF+x4Wpx+9twYpWxNLjzJMZFQMIg47tUJKcXRwtWIjIxn3qnUk9msbnMDtVzT1uy0KpYpmbItnaIMwqXw4EAhv1OjatUNQq5arW9iPl9wpkjDTEuE6AnduDOHBuyzZYYO/LnBjp9/u22UP6tXSUTcpYO1joQYPsTg5yfy4wWTFfZFTZBgWPJZbtD/nCI7/S99Csc43m/KRzz4d1qHTE7En4Yyyv6Mw8VvEO3l6rHzLfQ9vaS/SYh3ehqA63dWzT5Ie/qt/eJwk4/WmnGwb6ykGt7ExvDk1z3mvOvJNyxV1gp4g6c2zO+HJqtFeKWWx2xylTohO878ScRY6emiJgQFs5abV62dCx5a4hxsxgv9LyqG3stkMV8J7EWdF7uSWEbML8JgqXYuPUVe4aVRBBnctP364iJ21l3X23BdubI+xWZ+ZZ+dSpvelY+O0Cww98a4SeM3ckUB7Y/MUgVCntEObNIfN62e9RP4ddX7BU23LkKV0VgBmJt/uTT+9bHZGJgH+K6io58KzIWP24C3PNHpTkTbul+Y29iEGEJ+RSWMuL36la4JME5z8BeVA11JtWFVGE9MSGOzpcYW6SAlVUoT2TpXpy8u7MwLp+J0p3i1zXQ3sQgRyUcd+MMheAysPYQj1Xanv6DGG5zLlfYQIzbyp83MTg+Rg8WMY1nrrbLWV3Dl5FUbfvugqx9idka2w7GSfoSAHMWpePt076F9lPL7atrTyZibWY+8ibGHdCQD+fbvQ7+DRlyfmlcrVVUNDG6daPqi+EGmEUyESwaF77Jr2pEVekrDIm/NcG+7mcixuWF8iZW4WYyJaVmh45U/LT8zW3xn6ztnxORiG35/KGseT3zjqXzX+2ulfnNkslfudkDyGmQ8SpyfGKlT8b/6TvLGV6cmNhGdSzjv8h+grg8MdyrV737WjMruasSG/n0/jqyC9j2Jk6H0HrHJqPb7OuX/cYNXOpBK7Ncr4W+p6VxdkKnuZH87ahh9u+R1Me8ZXYBP7/T8lbXl5NE17OwobP/0yh7h4NuO4OrKyh1jZZTJ1aCCX0aOZcN1W1GaF+aeqv+JPn55WTSv5qb8lJzp9PFtB+vWPhrOrotB9BTpjZ+oSX9+wg6gbYu7VEsoztsQ7dCb4NCcaG7pkerpWnDuxEmNtxf0xawnfx8y1yBSrpQg472vYnhOYFdk2KldldjXx9jH+2eYym1u8LtDX8wMNOZ0VIHS/5EQ24i/p4OZKnd1F4Wg4z45nMApQ7WtBhkZNeVnVJ7qydviclDyrHcUjlqvm/AvAMwdcNcqRx1ZVq1OAiY/H3QUjkL27vxHmCT0iQWrTBPw6jd6FoqR83N317fR8hHRUsVoN5S//cN9uJFZiWwX6PpJI9RRnz8o5ylCtCof0jSXpSlfPe/VJGaO+b9ZFsHWFX+OEGpgjW9fIcDQkgzPHYF/j7ZXnSoabQ6s333wJXKQ+Or7iHDjLiXvdLlOI7s6czdlxmpdxqlT39EjYfuHi4IIfVmWHocR9Zo5fg7TWiEkO6k9Oh/B7W9ftO3zNSIRbpR6W/8Pmp4s6gj/905RsslzeGiV5rD30r2tJZQ2/y9Q7Z32XLr1eblal56G7+l2uNpax4OhxH9S4OT4czrTcej0pn/raX5Y4Slfon+DwBuoTARrmK5AAAAAElFTkSuQmCC"
                            /*MouseArea { //  This does not work :(
                                anchors.fill: parent
                                onClicked: Qt.openUrlExternally("https://revuo-xmr.com/issue-150.html") //...handling the clicked signal of the MouseArea
                                cursorShape: Qt.PointingHandCursor
                            }*/
                        //}
                    }
                }

                NeroshopComponents.BannerItem {
                    Rectangle {
                        id: secondPage
                        anchors.fill: parent
                        color: "red"
                    }
                }

                NeroshopComponents.BannerItem {
                    Rectangle {
                        id: thirdPage
                        anchors.fill: parent
                        color: "purple"
                    }
                }
            } // Banner
            
            Text {
                Layout.row: 2
                Layout.topMargin: 10
                text: "Categories"
                color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                font.bold: true
                font.pointSize: 16
            }
            
            Flow {
                Layout.row: 3
                Layout.preferredWidth: parent.width////scrollView.width
                Layout.maximumWidth: scrollView.width////mainWindow.width
                Layout.topMargin: 10
                spacing: 5
                //Layout.alignment: Qt.AlignHCenter | Qt.AlignTop // does nothing
                Repeater {
                    id: categoryRepeater
                    model: Backend.getCategoryList()
                    delegate: Rectangle {
                        implicitWidth: 150//200
                        implicitHeight: 90//implicitWidth / 2
                        color: NeroshopComponents.Style.getColorsFromTheme()[1]
                        radius: 5//3
                        border.color: hovered ? "white" : "transparent"
                        property bool hovered: false
                        // TODO: replace catalog tooltip with normal text
                        NeroshopComponents.Hint {/*TextArea {
                            id: categoryNameText
                            text: modelData.name
                            color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                            readOnly: true
                            wrapMode: TextEdit.Wrap
                            width: parent.width
                            verticalAlignment: TextEdit.AlignVCenter
                            anchors.top: parent.top
                            anchors.topMargin: 10*/
                            visible: parent.hovered
                            height: contentHeight + 20; width: contentWidth + 20
                            text: qsTr(modelData.name)
                            pointer.visible: false; delay: 0
                        }
                        Image {
                            id: categoryThumbnail
                            source: (modelData.thumbnail == "NULL") ? "qrc:/assets/images/image_gallery.png" : modelData.thumbnail
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top//anchors.bottom: categoryNameText.bottom
                            anchors.topMargin: 10//anchors.bottomMargin: 20////anchors.centerIn: parent
                            fillMode: Image.PreserveAspectFit
                            width: 32; height: 32
                        }
                        Rectangle {
                            anchors.bottom: parent.bottom//categoryRepeater.itemAt(index).children[0].bottom
                            anchors.bottomMargin: 10//20
                            anchors.horizontalCenter: parent.horizontalCenter
                            implicitWidth: 75
                            implicitHeight: 25
                            color: parent.color//"#000000"
                            radius: 5
                            border.color: "#ffffff"
                            Text {
                                text: Backend.getCategoryProductCount(modelData.id) // Number of products that fall under this particular category
                                color: "#ffffff"
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                        MouseArea { 
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.hovered = true
                            onExited: parent.hovered = false
                            onClicked: {
                                if(Backend.getCategoryProductCount(modelData.id) <= 0) return;
                                navBar.uncheckAllButtons()
                                pageLoader.setSource("qrc:/qml/pages/CatalogPage.qml", { "model": Backend.getListingsByCategory(modelData.id, settingsDialog.hideIllegalProducts) })
                            }
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
            }            
            // Recent listings
            Item { 
                Layout.row: 4
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 10
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                Layout.maximumWidth: scrollView.width
                visible: itemsRepeater.count > 0
                
                Column {
                    spacing: 10
                    Text {
                        text: "Recently added"//"Recent listings"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                        font.pointSize: 16
                    }
            
                    Flow {
                        width: parent.parent.parent.width////scrollView.width
                        spacing: 5
                        Repeater {
                            id: itemsRepeater
                            model: Backend.getListingsByMostRecentLimit(6, settingsDialog.hideIllegalProducts)
                            delegate: Rectangle {
                                implicitWidth: 200
                                implicitHeight: implicitWidth
                                color: NeroshopComponents.Style.getColorsFromTheme()[1]
                                radius: 3
                                Image {
                                    source: "image://listing?id=%1&image_id=%2".arg(modelData.key).arg(modelData.product_images[0].name)//"qrc:/assets/images/image_gallery.png"
                                    anchors.centerIn: parent
                                    width: parent.width - 10; height: parent.height - 10//width: 128; height: 128
                                    fillMode: Image.PreserveAspectFit
                                    mipmap: true
                                    //asynchronous: true
                                    MouseArea {
                                        anchors.fill: parent
                                        //hoverEnabled: true
                                        acceptedButtons: Qt.LeftButton
                                        onClicked: { 
                                            //navBar.uncheckAllButtons()
                                            pageLoader.setSource("qrc:/qml/pages/ProductPage.qml", { "model": modelData })
                                        }
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                            }
                        }
                    }
                }    
            }        
/*            Item {
                Layout.row: 5
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 10
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                Layout.maximumWidth: scrollView.width
                
                Column {
                    spacing: 10
                    Text {
                        text: "Recently added"//"Recent listings"//"Featured items"//"Best sellers"//"On Sale"//"Recommended items"//"Shop by Category"
                        color: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                        font.bold: true
                        font.pointSize: 16
                    }
            
                    Flow {
                        width: parent.parent.parent.width//scrollView.width
                        spacing: 5
                        Repeater {
                            id: itemsRepeater
                            model: 16//6
                            delegate: Rectangle {
                                implicitWidth: 200
                                implicitHeight: implicitWidth
                                color: NeroshopComponents.Style.getColorsFromTheme()[1]
                                radius: 3
                                Image {
                                    source: "qrc:/assets/images/image_gallery.png"
                                    anchors.centerIn: parent
                                }
                            }
                        }
                    }
                }    
            }*/
        } // GridLayout
    } // ScrollView
}
