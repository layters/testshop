import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.platform 1.1 // FileDialog

import FontAwesome 1.0

import "." as NeroshopComponents

Popup {
    id: productDialog
    implicitWidth: 800////parent.width
    implicitHeight: 500//mainWindow.height - (mainWindow.header.height + mainWindow.footer.height)////500    
    visible: false
    modal: true
    closePolicy: Popup.CloseOnEscape
    palette.text: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
    property string inputTextColor: palette.text
    property real titleSpacing: 10 // space between title and input fields/controls
    property string inputBaseColor: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#17171c" : "#1a1a1a") : "#fafafa"
    property string inputBorderColor: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"
    property real inputRadius: 10
    property string optTextColor: "#708090"
    
    background: Rectangle {
        radius: 8
        color: (NeroshopComponents.Style.darkTheme) ? (NeroshopComponents.Style.themeName == "PurpleDust" ? "#0e0e11" : "#101010") : "#f0f0f0"
    
        // header
        Rectangle {
           id: titleBar
           color: "transparent"
           height: 40
           width: parent.width
           anchors.left: parent.left
           anchors.right: parent.right
           radius: 6

           // Rounded top corners
           Rectangle {
               anchors.left: parent.left
               anchors.right: parent.right
               anchors.bottom: parent.bottom
               height: parent.height / 2
               color: parent.color
           }
    
           Button {
               id: closeButton
               width: 25
               height: this.width
    
               anchors.verticalCenter: titleBar.verticalCenter
               anchors.right: titleBar.right
               anchors.rightMargin: 10
               text: qsTr(FontAwesome.xmark)
               contentItem: Text {  
                   text: closeButton.text
                   color: "#ffffff"
                   font.bold: true
                   font.family: FontAwesome.fontFamily
                   horizontalAlignment: Text.AlignHCenter
                   verticalAlignment: Text.AlignVCenter
               }
               background: Rectangle {
                   color: "#ff4d4d"
                   radius: 100
               }
               onClicked: {
                   productDialog.close()
                   mainScrollView.ScrollBar.vertical.position = 0.0 // reset scrollbar
               }
           }
       }
    }
    
    contentItem: ScrollView {
        id: mainScrollView
        anchors.fill: parent
        anchors.topMargin: titleBar.height + 20; anchors.bottomMargin: 20//anchors.topMargin////anchors.margins: 20
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn//AsNeeded
        ColumnLayout {
            width: productDialog.availableWidth; height: productDialog.availableHeight
            spacing: 30
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                // Product title
                Column {
                    spacing: productDialog.titleSpacing
                    Text {
                        text: "Name or title"
                        color: productDialog.palette.text
                        font.bold: true
                    }
                            
                    TextField {
                        id: productNameField
                        width: 500; height: 50
                        placeholderText: qsTr("Enter name")
                        color: productDialog.inputTextColor
                        selectByMouse: true
                        maximumLength: 200
                        background: Rectangle { 
                            color: "transparent"
                            border.color: productDialog.inputBorderColor
                            border.width: parent.activeFocus ? 2 : 1
                            radius: productDialog.inputRadius
                        }
                    }
                }
            }
            // Product price (sales price)
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Price"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                x: parent.width + 10; y: ((parent.height - height) / 2) - 3
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("Price per unit")
                                pointer.visible: false;
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }

                    TextField {
                        id: productPriceField
                        width: 500/* - parent.children[1].width - parent.spacing*/; height: 50//Layout.preferredWidth: 500 - parent.children[1].width - parent.spacing; Layout.preferredHeight: 50
                        placeholderText: qsTr("Enter price")
                        color: productDialog.inputTextColor
                        selectByMouse: true
                        validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + Backend.getCurrencyDecimals(settingsDialog.currency.currentText) + "})?$") }
                        background: Rectangle { 
                            color: "transparent"
                            border.color: productDialog.inputBorderColor
                            border.width: parent.activeFocus ? 2 : 1
                            radius: productDialog.inputRadius
                        }
                        rightPadding: 25 + selectedCurrencyText.width
                        function adjustPriceDecimals() {
                            productPriceField.text = Number(productPriceField.text).toFixed(Backend.getCurrencyDecimals(settingsDialog.currency.currentText))
                        }
                        onEditingFinished: adjustPriceDecimals() // does not update when switching from crypto to fiat :(
                                    
                        Text {
                            id: selectedCurrencyText
                            text: settingsDialog.currency.currentText
                            color: productDialog.inputTextColor
                            anchors.right: parent.right
                            anchors.rightMargin: 15
                            anchors.verticalCenter: parent.verticalCenter
                            font.bold: true
                            font.pointSize: 10
                            MouseArea { 
                                anchors.fill: parent
                                onClicked: {
                                    settingsDialog.currentIndex = 0 // Switch to General Settings tab
                                    settingsDialog.open()
                                    settingsDialog.currency.popup.open()
                                }
                            }
                        }
                    }
                }
            }
            // Product quantity
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: qsTr("Quantity")
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            //font.pointSize: 
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                x: parent.width + 10; y: ((parent.height - height) / 2) - 3
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("Stock quantity")
                                pointer.visible: false;// delay: 0
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }
                            
                    TextField {
                        id: productQuantityField
                        width: 500; height: 50
                        placeholderText: qsTr("Enter quantity")
                        color: productDialog.inputTextColor
                        selectByMouse: true
                        inputMethodHints: Qt.ImhDigitsOnly // for Android and iOS - typically used for input of languages such as Chinese or Japanese
                        validator: RegExpValidator{ regExp: /[0-9]*/ }
                        text: "1"
                        background: Rectangle { 
                            color: "transparent"
                            border.color: productDialog.inputBorderColor
                            border.width: parent.activeFocus ? 2 : 1
                            radius: productDialog.inputRadius
                        }     
                        rightPadding: 25 + quantityIncreaseText.contentWidth
                        function adjustQuantity() {
                            if(Number(productQuantityField.text) >= quantityIncreaseText.maximumQuantity) {
                                productQuantityField.text = quantityIncreaseText.maximumQuantity
                            }
                            if(Number(productQuantityField.text) <= quantityDecreaseText.minimumQuantity) {
                                productQuantityField.text = quantityDecreaseText.minimumQuantity
                            }
                        }
                        onEditingFinished: adjustQuantity()
                                
                        Text {
                            id: quantityIncreaseText
                            text: qsTr("\uf0d8")
                            color: productDialog.inputTextColor
                            font.bold: true
                            anchors.right: parent.right
                            anchors.rightMargin: 15
                            anchors.top: parent.top; anchors.topMargin: 10
                            property real maximumQuantity: 999999999
                            MouseArea { 
                                anchors.fill: parent
                                onClicked: {
                                    if(Number(productQuantityField.text) >= parent.maximumQuantity) {
                                        productQuantityField.text = parent.maximumQuantity
                                        return;
                                    }
                                    productQuantityField.text = Number(productQuantityField.text) + 1
                                }
                            }
                        }    
                        Text {
                            id: quantityDecreaseText
                            text: qsTr("\uf0d7")
                            color: productDialog.inputTextColor
                            font.bold: true
                            anchors.right: parent.right
                            anchors.rightMargin: 15
                            anchors.bottom: parent.bottom; anchors.bottomMargin: 10
                            property real minimumQuantity: 1
                            MouseArea { 
                                anchors.fill: parent
                                onClicked: {
                                    if(Number(productQuantityField.text) <= parent.minimumQuantity) {
                                        productQuantityField.text = parent.minimumQuantity
                                        return;
                                    }
                                    productQuantityField.text = Number(productQuantityField.text) - 1
                                }
                            }
                        }
                    }
                    
                    Item {
                        width: 500; height: childrenRect.height
                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: quantityPerOrderField.verticalCenter
                            spacing: 10
                            Text {
                                text: qsTr("Quantity per order")
                                color: productDialog.palette.text
                                font.pointSize: 10
                            }
                            Text {
                                text: qsTr(FontAwesome.questionCircle)
                                color: productDialog.optTextColor
                                font.bold: true
                                //font.pointSize: 
                                anchors.verticalCenter: parent.children[0].verticalCenter
                                property bool hovered: false
                                NeroshopComponents.Hint {
                                    x: parent.width + 10; y: ((parent.height - height) / 2)
                                    visible: parent.hovered
                                    height: contentHeight + 20; width: contentWidth + 20
                                    text: qsTr("Limit the quantity a buyer can purchase for each order")
                                    pointer.visible: false;
                                }
                                MouseArea { 
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: parent.hovered = true
                                    onExited: parent.hovered = false
                                }
                            }
                        }
                        TextField {
                            id: quantityPerOrderField
                            anchors.right: parent.right
                            width: 50; height: 25
                            placeholderText: qsTr("max")
                            font.pixelSize: 12
                            color: productDialog.inputTextColor
                            selectByMouse: true
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: RegExpValidator{ regExp: /[0-9]*/ }
                            maximumLength: 2
                            onEditingFinished: {
                                if (text === "00") text = "";
                                if (text.startsWith("0")) text = text.substring(1);
                            }
                            background: Rectangle { 
                                color: "transparent"
                                border.color: productDialog.inputBorderColor
                                border.width: parent.activeFocus ? 2 : 1
                                radius: productDialog.inputRadius
                            }
                        }
                    }
                }
            }                                
            // Product condition
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Text {
                        text: "Condition"
                        color: productDialog.palette.text
                        font.bold: true
                        //visible: false
                    }
                    NeroshopComponents.ComboBox {
                        id: productConditionBox
                        width: 500; height: 50
                        model: ["New", "Used", "Refurbished (Renewed)", "Not applicable"] // default is New
                        Component.onCompleted: currentIndex = find("New")
                        radius: productDialog.inputRadius
                        color: productDialog.inputBaseColor
                        textColor: productDialog.inputTextColor
                    }
                }
            }
            // Product code UPC, EAN, JAN, SKU, ISBN (for books) // https://www.simplybarcodes.com/barcode_types.html
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                        
                Column {
                    spacing: productDialog.titleSpacing
                            
                    Row {
                        spacing: 10
                        Text {
                            text: "Product code"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: "(OPTIONAL)"
                            color: productDialog.optTextColor
                            font.bold: true
                            font.pointSize: 8
                            anchors.verticalCenter: parent.children[0].verticalCenter
                        }
                    }
                            
                    Row {
                        spacing: 5
                        TextField {
                            id: productCodeField
                            width: 500 - parent.children[1].width - parent.spacing; height: 50
                            placeholderText: qsTr("Enter product code")
                            color: productDialog.inputTextColor
                            selectByMouse: true
                            background: Rectangle { 
                                color: "transparent"
                                border.color: productDialog.inputBorderColor
                                border.width: parent.activeFocus ? 2 : 1
                                radius: productDialog.inputRadius
                            }                            
                        }
                        NeroshopComponents.ComboBox {
                            id: productCodeType
                            height: parent.children[0].height//Layout.preferredWidth: 100; Layout.preferredHeight: parent.children[0].height
                            model: ["EAN", "ISBN", "JAN", "SKU", "UPC"] // default is UPC (each code will be validated before product is listed)
                            Component.onCompleted: currentIndex = find("UPC")
                            radius: productDialog.inputRadius
                            color: productDialog.inputBaseColor
                            textColor: productDialog.inputTextColor
                        }
                    }
                }
            }
            // Product categories
            Item {
                //Layout.row: 
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                function getCategoryStringList() {
                    let categoryStringList = []
                    let categories = Backend.getCategoryList(true)
                    for(let i = 0; i < categories.length; i++) {
                        categoryStringList[i] = categories[i].name//console.log(parent.parent.parent.categoryStringList[i])//console.log(categories[i].name)
                    }       
                    return categoryStringList;
                }
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Text {
                        text: "Category"
                        color: productDialog.palette.text
                        font.bold: true
                    }
                            
                    Row {
                        spacing: 5
                        NeroshopComponents.ComboBox {
                            id: productCategoryBox
                            width: addSubCategoryButton.visible ? (500 - addSubCategoryButton.width - parent.spacing) : 500; height: 50
                            model: parent.parent.parent.getCategoryStringList()
                            Component.onCompleted: {
                                currentIndex = find("Miscellaneous")
                            }
                            function reset() {
                                let subcategories = Backend.getSubCategoryList(Backend.getCategoryIdByName(productCategoryBox.currentText), true)
                                addSubCategoryButton.visible = (subcategories.length > 0)
                                subCategoryRepeater.model = 0 // reset
                            }
                            onActivated: {
                                productCategoryBox.reset()
                            }
                            radius: productDialog.inputRadius
                            color: productDialog.inputBaseColor
                            textColor: productDialog.inputTextColor
                        }
                        Button {
                            id: addSubCategoryButton
                            width: 50; height: 50
                            text: qsTr("+")
                            hoverEnabled: true
                            visible: Backend.hasSubCategory(Backend.getCategoryIdByName(productCategoryBox.currentText))
                            background: Rectangle {
                                color: parent.hovered ? "#698b22" : "#506a1a"//"#605185"
                                radius: productDialog.inputRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                            onClicked: {
                                let subcategories = Backend.getSubCategoryList(Backend.getCategoryIdByName(productCategoryBox.currentText), true)
                                let max_subcategories = Math.min(2, subcategories.length)
                                if(subCategoryRepeater.count == max_subcategories) {
                                    console.log("Cannot add no more than " + max_subcategories + " subcategories")
                                    return
                                }
                                subCategoryRepeater.model = subCategoryRepeater.model + 1
                            }
                        }
                    }
                }
            }                    
            // Subcategories (will be determined based on selected categories)
            Item {
                id: subCategoryItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                visible: (subCategoryRepeater.count > 0)//Backend.hasSubCategory(Backend.getCategoryIdByName(productCategoryBox.currentText))
                        
                function getSubCategoryStringList() {
                    let subCategoryStringList = []
                    let subcategories = Backend.getSubCategoryList(Backend.getCategoryIdByName(productCategoryBox.currentText), true)
                    for(let i = 0; i < subcategories.length; i++) {
                        subCategoryStringList[i] = subcategories[i].name//console.log(parent.parent.parent.categoryStringList[i])//console.log(categories[i].name)
                    }       
                    return subCategoryStringList;
                }
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Text {
                        text: "Subcategory"
                        color: productDialog.palette.text
                        font.bold: true
                    }
                            
                    Repeater {
                        id: subCategoryRepeater
                        model: 0
                        delegate: Row {
                            spacing: 5
                            NeroshopComponents.ComboBox {
                                id: productSubCategoryBox
                                width: removeSubCategoryButton.visible ? (500 - removeSubCategoryButton.width - parent.spacing) : 500; height: 50
                                model: parent.parent.parent.getSubCategoryStringList()
                                currentIndex: 0
                                radius: productDialog.inputRadius
                                color: productDialog.inputBaseColor
                                textColor: productDialog.inputTextColor
                            }
                            Button {
                                id: removeSubCategoryButton
                                width: 50; height: 50
                                text: qsTr("x")
                                hoverEnabled: true
                                background: Rectangle {
                                    color: parent.hovered ? "#b22222" : "#921c1c"
                                    radius: productDialog.inputRadius
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#ffffff"
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: {
                                    subCategoryRepeater.model = subCategoryRepeater.model - 1
                                }
                            }
                        } // Row
                    }
                }
            }
            // Weight
            Item {
                //Layout.row: 
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Weight"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: "(OPTIONAL)"
                            color: productDialog.optTextColor
                            font.bold: true
                            font.pointSize: 8
                            anchors.verticalCenter: parent.children[0].verticalCenter
                        }                            
                    }
                            
                    Row {
                        spacing: 5
                        TextField {
                            id: productWeightField
                            width: 500 - parent.children[1].width - parent.spacing; height: 50
                            placeholderText: qsTr("Enter weight")
                            color: productDialog.inputTextColor
                            selectByMouse: true
                            validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + 8 + "})?$") }
                            background: Rectangle { 
                                color: "transparent"
                                border.color: productDialog.inputBorderColor
                                border.width: parent.activeFocus ? 2 : 1
                                radius: productDialog.inputRadius
                            }
                        }
                        NeroshopComponents.ComboBox {
                            id: weightMeasurementUnit
                            height: parent.children[0].height
                            model: ["kg", "lb"] // default is kg (every unit of measurement will be converted to kg)
                            Component.onCompleted: currentIndex = find("kg")
                            radius: productDialog.inputRadius
                            color: productDialog.inputBaseColor
                            textColor: productDialog.inputTextColor
                        }
                    }
                }
            }                    
            // Size
            /*Item {
                  //Layout.row: 
                  Layout.alignment: Qt.AlignHCenter
                  Layout.preferredWidth: childrenRect.width
                  Layout.preferredHeight: childrenRect.height
                        
                  Column {
                      spacing: productDialog.titleSpacing
                      Row {
                          spacing: 10
                          Text {
                              text: "Size"
                              color: productDialog.palette.text
                              font.bold: true
                          }
                          Text {
                              text: "(OPTIONAL)"
                              color: productDialog.optTextColor
                              font.bold: true
                              font.pointSize: 8
                              anchors.verticalCenter: parent.children[0].verticalCenter
                          }
                      }
                         
                      Row {
                          spacing: 5
                          TextField {
                              id: productSizeField
                              width: 500; height: 50
                              placeholderText: qsTr("Enter size")
                              color: productDialog.inputTextColor
                              selectByMouse: true
                              background: Rectangle { 
                                  color: "transparent"
                                  border.color: productDialog.inputBorderColor
                                  border.width: parent.activeFocus ? 2 : 1
                                  radius: productDialog.inputRadius
                              }                            
                          }
                          //ComboBox
                     }
                 }
             }*/                    
            // Variations/Attributes (i.e. Color, Size, Type, Model, etc. options to choose from - optional)
            // Product location (ship to and ship from)
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                property var countriesModel: ["Afghanistan", "Albania", "Algeria", "Andorra", "Angola", "Antigua & Deps", "Argentina", "Armenia", "Australia", "Austria", "Azerbaijan", "Bahamas", "Bahrain", "Bangladesh", "Barbados", "Belarus", "Belgium", "Belize", "Benin", "Bermuda", "Bhutan", "Bolivia", "Bosnia Herzegovina", "Botswana", "Brazil", "Brunei", "Bulgaria", "Burkina", "Burundi", "Cambodia", "Cameroon", "Canada", "Cape Verde", "Central African Rep", "Chad", "Chile", "China", "Colombia", "Comoros", "Congo", "Congo (Democratic Rep)", "Costa Rica", "Croatia", "Cuba", "Cyprus", "Czech Republic", "Denmark", "Djibouti", "Dominica", "Dominican Republic", "East Timor", "Ecuador", "Egypt", "El Salvador", "Equatorial Guinea", "Eritrea", "Estonia", "Eswatini", "Ethiopia", "Fiji", "Finland", "France", "Gabon", "Gambia", "Georgia", "Germany", "Ghana", "Greece", "Grenada", "Guatemala", "Guinea", "Guinea-Bissau", "Guyana", "Haiti", "Honduras", "Hungary", "Iceland", "India", "Indonesia", "Iran", "Iraq", "Ireland (Republic)", "Israel", "Italy", "Ivory Coast", "Jamaica", "Japan", "Jordan", "Kazakhstan", "Kenya", "Kiribati", "Korea North", "Korea South", "Kosovo", "Kuwait", "Kyrgyzstan", "Laos", "Latvia", "Lebanon", "Lesotho", "Liberia", "Libya", "Liechtenstein", "Lithuania", "Luxembourg", "Macedonia", "Madagascar", "Malawi", "Malaysia", "Maldives", "Mali", "Malta", "Marshall Islands", "Mauritania", "Mauritius", "Mexico", "Micronesia", "Moldova", "Monaco", "Mongolia", "Montenegro", "Morocco", "Mozambique", "Myanmar", "Namibia", "Nauru", "Nepal", "Netherlands", "New Zealand", "Nicaragua", "Niger", "Nigeria", "Norway", "Oman", "Online", "Pakistan", "Palau", "Palestine", "Panama", "Papua New Guinea", "Paraguay", "Peru", "Philippines", "Poland", "Portugal", "Qatar", "Romania", "Russian Federation", "Rwanda", "St Kitts & Nevis", "St Lucia", "Saint Vincent & the Grenadines", "Samoa", "San Marino", "Sao Tome & Principe", "Saudi Arabia", "Senegal", "Serbia", "Seychelles", "Sierra Leone", "Singapore", "Slovakia", "Slovenia", "Solomon Islands", "Somalia", "South Africa", "South Sudan", "Spain", "Sri Lanka", "Sudan", "Suriname", "Sweden", "Switzerland", "Syria", "Taiwan", "Tajikistan", "Tanzania", "Thailand", "Togo", "Tonga", "Trinidad & Tobago", "Tunisia", "Turkey", "Turkmenistan", "Tuvalu", "Uganda", "Ukraine", "United Arab Emirates", "United Kingdom", "United States", "Unspecified", "Uruguay", "Uzbekistan", "Vanuatu", "Vatican City", "Venezuela", "Vietnam", "Yemen", "Zambia", "Zimbabwe", "Worldwide",]
                        
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Location"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                x: parent.width + 10; y: ((parent.height - height) / 2) - 3
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("Ships from?")
                                pointer.visible: false;
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }
                            
                    NeroshopComponents.ComboBox {
                        id: productLocationBox
                        width: 500; height: 50
                        model: parent.parent.countriesModel
                        Component.onCompleted: {
                            contentItem.selectByMouse = true
                            currentIndex = find("Unspecified")//find("Worldwide")
                        }
                        editable: true
                        onAccepted: {
                            if(find(editText) === -1)
                                model.append({text: editText})
                        }
                        radius: productDialog.inputRadius
                        color: productDialog.inputBaseColor
                        textColor: productDialog.inputTextColor
                    }
                }
            } 
            // Seller-accepted payment options
            Item {
                id: paymentOptionsItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                property var paymentOptions: [
                    { name: "Escrow", value: 0 },
                    { name: "Multisig", value: 1 },
                    { name: "Finalize", value: 2 }
                ]
                function isPaymentOptionSelected() {
                    // Check if at least one checkbox is checked
                    for (let i = 0; i < paymentOptionsRepeater.count; ++i) {
                        if (paymentOptionsRepeater.itemAt(i).checked) {
                            return true;
                        }
                    }
                    return false;
                }
                function getSelectedPaymentOptions() {
                    let selectedNames = [];
                    for (let i = 0; i < paymentOptionsRepeater.count; ++i) {
                        if (paymentOptionsRepeater.itemAt(i).checked) {
                            selectedNames.push(paymentOptionsRepeater.itemAt(i).text);
                        }
                    }
                    return selectedNames;
                }
                function getSelectedPaymentOptionsValue() {
                    let selectedValues = [];
                    for (let i = 0; i < paymentOptionsRepeater.count; ++i) {
                        if (paymentOptionsRepeater.itemAt(i).checked) {
                            selectedValues.push(paymentOptionsRepeater.itemAt(i).value);
                        }
                    }
                    return selectedValues;
                }
                function uncheckAllPaymentOptions() {
                    for (let i = 0; i < paymentOptionsRepeater.count; ++i) {
                        paymentOptionsRepeater.itemAt(i).checked = false
                    }
                }
    
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Payment options"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                //x: parent.width + 10; y: ((parent.height - height) / 2)
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("-- Select all that apply --\nEscrow: 2 of 3 Multisig\nMultisig: 2 of 2 Multisig\nFinalize: Direct payment")
                                pointer.visible: false;
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }
                    // Checkboxes for payment options
                    Frame {
                        width: 500; height: 30//50//80 - for Flow
                        background: Rectangle {
                            color: "transparent"//productDialog.inputBaseColor
                            //border.color: productDialog.inputBorderColor
                            radius: productDialog.inputRadius
                        }
                        Row {//Flow {
                            anchors.fill: parent/*width: width + (paymentOptionsRepeater.count * spacing); height: parent.height
                            anchors.horizontalCenter: parent.horizontalCenter*/
                            spacing: 100
                            Repeater {
                                id: paymentOptionsRepeater
                                model: paymentOptionsItem.paymentOptions
                                delegate: NeroshopComponents.CheckBox {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.name
                                    textColor: productDialog.inputTextColor
                                    color: "transparent"
                                    property int value: modelData.value
                                }
                            }
                        }
                    }
                }
            }
            // Seller-accepted payment coins
            Item {
                id: paymentCoinsItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                property var paymentCoins: [
                    { name: "Monero", selected: true, value: 0 }/*,
                    { name: "Wownero", selected: false, value: 1 }*/
                ]
                function isPaymentCoinSelected() {
                    // Check if at least one checkbox is checked
                    for (let i = 0; i < paymentCoinsRepeater.count; ++i) {
                        if (paymentCoinsRepeater.itemAt(i).checked) {
                            return true;
                        }
                    }
                    return false;
                }
                function getSelectedPaymentCoins() {
                    let selectedNames = [];
                    for (let i = 0; i < paymentCoinsRepeater.count; ++i) {
                        if (paymentCoinsRepeater.itemAt(i).checked) {
                            selectedNames.push(paymentCoinsRepeater.itemAt(i).text);
                        }
                    }
                    return selectedNames;
                }
                function getSelectedPaymentCoinsValue() {
                    let selectedValues = [];
                    for (let i = 0; i < paymentCoinsRepeater.count; ++i) {
                        if (paymentCoinsRepeater.itemAt(i).checked) {
                            selectedValues.push(paymentCoinsRepeater.itemAt(i).value);
                        }
                    }
                    return selectedValues;
                }
                function uncheckAllPaymentCoins() {
                    for (let i = 0; i < paymentCoinsRepeater.count; ++i) {
                        paymentCoinsRepeater.itemAt(i).checked = false
                    }
                }
    
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Payment coins"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("-- Select all that apply --")
                                pointer.visible: false;
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }
                    // Checkboxes for payment coins
                    Frame {
                        width: 500; height: 30//50//80 - for Flow
                        background: Rectangle {
                            color: "transparent"//productDialog.inputBaseColor
                            //border.color: productDialog.inputBorderColor
                            radius: productDialog.inputRadius
                        }
                        Row {//Flow {
                            anchors.fill: parent
                            spacing: 100
                            Repeater {
                                id: paymentCoinsRepeater
                                model: paymentCoinsItem.paymentCoins
                                delegate: NeroshopComponents.CheckBox {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.name
                                    checked: modelData.selected
                                    textColor: productDialog.inputTextColor
                                    color: "transparent"
                                    property int value: modelData.value
                                }
                            }
                        }
                    }
                }
            }
            // Seller-accepted delivery options
            Item {
                id: deliveryOptionsItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                property var deliveryOptions: [
                    { name: "Shipping", selected: false, value: 0 },
                    { name: "Pickup", selected: false, value: 1 },
                    { name: "Digital", selected: (productLocationBox.contentItem.text == "Online") ? true : false, value: 2 }
                ]
                function isDeliveryOptionSelected() {
                    // Check if at least one checkbox is checked
                    for (let i = 0; i < deliveryOptionsRepeater.count; ++i) {
                        if (deliveryOptionsRepeater.itemAt(i).checked) {
                            return true;
                        }
                    }
                    return false;
                }
                function getSelectedDeliveryOptions() {
                    let selectedNames = [];
                    for (let i = 0; i < deliveryOptionsRepeater.count; ++i) {
                        if (deliveryOptionsRepeater.itemAt(i).checked) {
                            selectedNames.push(deliveryOptionsRepeater.itemAt(i).text);
                        }
                    }
                    return selectedNames;
                }
                function getSelectedDeliveryOptionsValue() {
                    let selectedValues = [];
                    for (let i = 0; i < deliveryOptionsRepeater.count; ++i) {
                        if (deliveryOptionsRepeater.itemAt(i).checked) {
                            selectedValues.push(deliveryOptionsRepeater.itemAt(i).value);
                        }
                    }
                    return selectedValues;
                }
                function uncheckAllDeliveryOptions() {
                    for (let i = 0; i < deliveryOptionsRepeater.count; ++i) {
                        deliveryOptionsRepeater.itemAt(i).checked = false
                    }
                }
    
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Delivery options"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("-- Select all that apply --")
                                pointer.visible: false;
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }
                    // Checkboxes for delivery options
                    Frame {
                        width: 500; height: 30//50//80 - for Flow
                        background: Rectangle {
                            color: "transparent"//productDialog.inputBaseColor
                            //border.color: productDialog.inputBorderColor
                            radius: productDialog.inputRadius
                        }
                        Row {//Flow {
                            anchors.fill: parent
                            spacing: 100
                            Repeater {
                                id: deliveryOptionsRepeater
                                model: deliveryOptionsItem.deliveryOptions
                                delegate: NeroshopComponents.CheckBox {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.name
                                    checked: modelData.selected
                                    textColor: productDialog.inputTextColor
                                    color: "transparent"
                                    property int value: modelData.value
                                }
                            }
                        }
                    }
                }
            }
            // Seller-accepted shipping options
            Item {
                id: shippingOptionsItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                visible: deliveryOptionsRepeater.count > 0 && deliveryOptionsRepeater.itemAt(0).checked
                property var shippingOptions: [
                    { name: "Standard", selected: true, value: 0 },
                    { name: "Expedited", selected: false, value: 1 }, // Priority
                    { name: "Express", selected: false, value: 2 },
                    { name: "Overnight", selected: false, value: 3 }, // NextDay
                    { name: "LocalDelivery", selected: false, value: 4 }, // SameDay
                    { name: "International", selected: false, value: 5 },
                    { name: "EcoFriendly", selected: false, value: 6 }
                ]
                function isShippingOptionSelected() {
                    // Check if at least one checkbox is checked
                    for (let i = 0; i < shippingOptionsRepeater.count; ++i) {
                        if (shippingOptionsRepeater.itemAtIndex(i).children[0].checked) {
                            return true;
                        }
                    }
                    return false;
                }
                function getSelectedShippingOptions() {
                    let selectedNames = [];
                    for (let i = 0; i < shippingOptionsRepeater.count; ++i) {
                        if (shippingOptionsRepeater.itemAtIndex(i).children[0].checked) {
                            selectedNames.push(shippingOptionsRepeater.itemAtIndex(i).children[0].text);
                        }
                    }
                    return selectedNames;
                }
                function getSelectedShippingOptionsValue() {
                    let selectedValues = [];
                    for (let i = 0; i < shippingOptionsRepeater.count; ++i) {
                        if (shippingOptionsRepeater.itemAtIndex(i).children[0].checked) {
                            selectedValues.push(shippingOptionsRepeater.itemAtIndex(i).children[0].value);
                        }
                    }
                    return selectedValues;
                }
                function uncheckAllShippingOptions() {
                    for (let i = 0; i < shippingOptionsRepeater.count; ++i) {
                        shippingOptionsRepeater.itemAtIndex(i).children[0].checked = false
                    }
                }
    
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Shipping options"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: qsTr(FontAwesome.questionCircle)
                            color: productDialog.optTextColor
                            font.bold: true
                            anchors.verticalCenter: parent.children[0].verticalCenter
                            property bool hovered: false
                            NeroshopComponents.Hint {
                                visible: parent.hovered
                                height: contentHeight + 20; width: contentWidth + 20
                                text: qsTr("-- Select all that apply --")
                                pointer.visible: false;
                            }
                            MouseArea { 
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.hovered = true
                                onExited: parent.hovered = false
                            }
                        }
                    }
                    // Checkboxes for shipping options
                    Frame {
                        width: 500; height: 95//70
                        background: Rectangle {
                            color: "transparent"//productDialog.inputBaseColor
                            //border.color: productDialog.inputBorderColor
                            radius: productDialog.inputRadius
                        }
                        GridView {//Grid
                            anchors.fill: parent
                            /*columns: 3
                            rows: 3
                            rowSpacing: 5
                            //columnSpacing: 30*/ // <- for regular Grid
                            cellWidth: 140
                            cellHeight: 20 + 5 // 20=checkbox height, 5=spacing
                            interactive: false
                            //Repeater {
                                id: shippingOptionsRepeater
                                model: shippingOptionsItem.shippingOptions
                                delegate: Item {
                                    width: childrenRect.width + (children[0].contentItem.contentWidth + children[0].contentItem.leftPadding)
                                    height: childrenRect.height
                                    NeroshopComponents.CheckBox {
                                        text: modelData.name
                                        checked: modelData.selected
                                        textColor: productDialog.inputTextColor
                                        color: "transparent"
                                        property int value: modelData.value
                                    }
                                }
                            //}
                        }
                    }
                }
            }
            // Seller-defined shipping costs for shipping option(s)
            Item {
                id: shippingCostsItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                visible: shippingOptionsItem.visible && shippingOptionsRepeater.count > 0 && shippingOptionsItem.isShippingOptionSelected()
                function getShippingCosts() {
                    let shippingCosts = [];
                    for (let i = 0; i < shippingCostsList.count; i++) {
                        let item = shippingCostsList.itemAtIndex(i);
                        let cost = item.children[0].children[1].text
                        if(Number(cost) > 0.00 && cost.length > 0) {
                            let shippingCostsObj = {
                                "shipping_option_str": item.children[0].children[0].text,
                                "shipping_option": item.children[0].children[0].value,
                                "cost": Number(cost)
                            };
                            shippingCosts.push(shippingCostsObj);
                            console.log("Shipping option:", shippingCostsObj.shipping_option_str);
                            console.log("Cost:", shippingCostsObj.cost);
                        }
                    }
                    return shippingCosts;
                }
                
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Shipping costs"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                    }
                    // ListView for shipping costs
                    Item {
                        width: 500; height: children[0].contentHeight
                        
                        ListView {
                            id: shippingCostsList
                            anchors.fill: parent//width: parent.width
                            spacing: 5
                            model: !shippingCostsItem.visible ? [] : shippingOptionsItem.getSelectedShippingOptionsValue()
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 50
                                color: "transparent"
                                border.color: productDialog.inputBorderColor
                                radius: productDialog.inputRadius
                                
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10 // horizontal margins
                                    spacing: 10
                                    
                                    Text {
                                        text: Backend.getShippingOptionAsString(modelData)
                                        color: productDialog.inputTextColor
                                        property int value: modelData
                                    }
                                    
                                    TextField {
                                        Layout.fillWidth: true
                                        placeholderText: qsTr(Number("0.00").toFixed(2))
                                        validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + Backend.getCurrencyDecimals(selectedCurrencyText.text) + "})?$") }
                                        selectByMouse: true
                                        background: Rectangle {
                                            color: "transparent"
                                            border.color: productDialog.inputBorderColor
                                            radius: productDialog.inputRadius
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // Seller-defined custom exchange rates for payment coin(s)
            Item {
                id: customRatesItem
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                visible: paymentCoinsRepeater.count > 0 && paymentCoinsItem.isPaymentCoinSelected() && selectedCurrencyText.text != "XMR"
                function getCustomRates() {
                    let customRates = [];
                    for (let i = 0; i < customRatesList.count; i++) {
                        let item = customRatesList.itemAtIndex(i);
                        let rate = item.children[0].children[1].text
                        if(Number(rate) > 0.00 && rate.length > 0) {
                            let customRatesObj = {
                                "payment_coin_str": item.children[0].children[0].text,
                                "payment_coin": item.children[0].children[0].value,
                                "rate": Number(rate)
                            };
                            customRates.push(customRatesObj);
                            console.log("Payment coin:", customRatesObj.payment_coin_str);
                            console.log("Rate:", customRatesObj.rate);
                        }
                    }
                    return customRates;
                }
                
                Column {
                    spacing: productDialog.titleSpacing
                    Row {
                        spacing: 10
                        Text {
                            text: "Custom rates"
                            color: productDialog.palette.text
                            font.bold: true
                        }
                        Text {
                            text: "(OPTIONAL)"
                            color: productDialog.optTextColor
                            font.bold: true
                            font.pointSize: 8
                            anchors.verticalCenter: parent.children[0].verticalCenter
                        }
                    }
                    // ListView for custom rates
                    Item {
                        width: 500; height: children[0].contentHeight
                        
                        ListView {
                            id: customRatesList
                            anchors.fill: parent//width: parent.width
                            spacing: 5
                            model: !customRatesItem.visible ? [] : paymentCoinsItem.getSelectedPaymentCoinsValue()
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 50
                                color: "transparent"
                                border.color: productDialog.inputBorderColor
                                radius: productDialog.inputRadius
                                
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10 // horizontal margins
                                    spacing: 10
                                    
                                    Text {
                                        text: Backend.getPaymentCoinAsString(modelData)
                                        color: productDialog.inputTextColor
                                        property int value: modelData
                                    }
                                    
                                    TextField {
                                        Layout.fillWidth: true
                                        placeholderText: qsTr("1 XMR = ? %1").arg(selectedCurrencyText.text)
                                        validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + Backend.getCurrencyDecimals(selectedCurrencyText.text) + "})?$") }
                                        selectByMouse: true
                                        background: Rectangle {
                                            color: "transparent"
                                            border.color: productDialog.inputBorderColor
                                            radius: productDialog.inputRadius
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
                    //Product description and bullet points
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDialog.titleSpacing
                            Text {
                                text: "Description"
                                color: productDialog.palette.text
                                font.bold: true
                            }
                            
                            ScrollView {
                                width: 500; height: 250
                                TextArea {
                                    id: productDescriptionEdit
                                    placeholderText: qsTr("Enter description")
                                    wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width//Text.WordWrap does not move text to the newline but instead it only shows the scrollbar
                                    selectByMouse: true
                                    color: productDialog.inputTextColor
                            
                                    background: Rectangle {
                                        color: "transparent"
                                        border.color: productDialog.inputBorderColor
                                        border.width: parent.activeFocus ? 2 : 1
                                        radius: productDialog.inputRadius
                                    }
                                }
                            }
                        }
                    }         
                    // Product tags
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDialog.titleSpacing
                            Text {
                                text: "Tags"
                                color: productDialog.palette.text
                                font.bold: true
                            }
                        
                            NeroshopComponents.TagField {
                                id: productTagsField
                                width: 500
                                
                                textField.color: productDialog.inputTextColor
                                textField.selectByMouse: true
                                textField.background: Rectangle { 
                                    color: "transparent"
                                    border.color: productDialog.inputBorderColor
                                    border.width: parent.activeFocus ? 2 : 1
                                    radius: productDialog.inputRadius
                                }
                            }
                        }
                    }
                    //Product images
                    Item {
                        Layout.alignment: Qt.AlignHCenter//Qt.AlignRight
                        Layout.preferredWidth: childrenRect.width//Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDialog.titleSpacing
                            Text {
                                text: "Upload image(s)"
                                color: productDialog.palette.text
                                font.bold: true
                            }
                            
                            Flickable {
                                width: 500; height: 210 + ScrollBar.horizontal.height// same height as delegateRect + scrollbar height
                                contentWidth: (210 * productImageRepeater.count) + (5 * (productImageRepeater.count - 1))//<- 5 is the Flow.spacing//; contentHeight: 210//<- contentHeight is not needed unless a newline is supported
                                clip: true
                                ScrollBar.horizontal: ScrollBar {
                                    policy: ScrollBar.AlwaysOn//AsNeeded
                                }
                                Flow {
                                    width: parent.contentWidth; height: parent.height//anchors.fill: parent// Note: Flow width must be large enough to fit all items horizontally so that there won't be a need to move an item to a newline
                                    spacing: 5
                                    Repeater {
                                        id: productImageRepeater
                                        model: 6
                                        property int lastImageIndex: -1
                                        delegate: Rectangle {//Item { 
                                            width: 210; height: 210
                                            color: "transparent"
                                            //border.color: "blue"
                                        
                                            Rectangle {
                                                border.color: productDialog.palette.text
                                                anchors.top: parent.top
                                                anchors.topMargin: 5
                                                anchors.horizontalCenter: parent.horizontalCenter
                                                width: parent.width; height: parent.height - 50
                                                color: "transparent"//"#d3d3d3"//
                                                Image {
                                                    anchors.centerIn: parent // This is not necessary since the image is the same size as its parent rect but I'll keep it there just to be sure the image is centered
                                                    width: parent.width; height: parent.height
                                                    fillMode: Image.PreserveAspectFit // scale to fit
                                                    ////source: productImageFileDialog.file
                                                    mipmap: true // produces better image quality that is not pixely but smooth :D
                                                    //asynchronous: true // won't block the app
                                                    onStatusChanged: {
                                                        if(this.status == Image.Ready) {
                                                            console.log(source + ' has been loaded')
                                                            // TODO: Upload image to the database
                                                            productImageRepeater.lastImageIndex = index // Update the lastImageIndex
                                                        }
                                                        if(this.status == Image.Loading) console.log("Loading image " + source + " (" + (progress * 100) + "%)")
                                                        if(this.status == Image.Error) console.log("An error occurred while loading the image")
                                                        //if(this.status == Image.Null) console.log("No image has been set" + parent.parent.index)
                                                    }
                                                }
                                                // Position the close button
                                                Button {
                                                    id: removeImageButton
                                                    anchors.right: parent.right
                                                    anchors.top: parent.top
                                                    anchors.margins: 8
                         
                                                    width: 20; height: 20//32
                                                    text: qsTr(FontAwesome.xmark)
                                                    hoverEnabled: true
                                                    visible: (parent.children[0].status === Image.Ready) && (index === productImageRepeater.lastImageIndex)
                            
                                                    contentItem: Text {
                                                        horizontalAlignment: Text.AlignHCenter
                                                        verticalAlignment: Text.AlignVCenter
                                                        text: removeImageButton.text
                                                        color: removeImageButton.hovered ? "#ffffff" : "#000000"
                                                        font.bold: true
                                                        font.family: FontAwesome.fontFamily
                                                    }
                        
                                                    background: Rectangle {
                                                        width: parent.width
                                                        height: parent.height
                                                        radius: 5//50
                                                        color: removeImageButton.hovered ? "firebrick" : "transparent"
                                                        opacity: 0.7
                                                    }
                         
                                                     onClicked: {
                                                         parent.children[0].source = ""
                                                         productImageRepeater.lastImageIndex = productImageRepeater.lastImageIndex - 1
                                                     }
                                                     MouseArea {
                                                        anchors.fill: parent
                                                        onPressed: mouse.accepted = false
                                                        cursorShape: Qt.PointingHandCursor
                                                    }
                                                }
                                            }
                                        
                                            Button {
                                                id: chooseFileButton
                                                width: 150; height: contentItem.contentHeight + 12
                                                text: qsTr("Choose File")// %1%").arg(parent.children[0].children[0].progress * 100)
                                                anchors.bottom: parent.bottom
                                                anchors.bottomMargin: 5
                                                anchors.horizontalCenter: parent.children[0].horizontalCenter
                                                background: Rectangle {
                                                    color: parent.hovered ? "lightslategray" : "slategray"
                                                    radius: productDialog.inputRadius
                                                }
                                                contentItem: Text {
                                                    text: parent.text
                                                    color: parent.hovered ? "#d9d9d9" : "#262626"
                                                    verticalAlignment: Text.AlignVCenter
                                                    horizontalAlignment: Text.AlignHCenter
                                                }
                                                onClicked: {
                                                    if(index != 0) {
                                                        let prevProductImage = productImageRepeater.itemAt(index - 1).children[0].children[0] // get image at previous index
                                                        if(prevProductImage.status == Image.Null) { console.log("Images must be loaded in order from left to right"); return; } // If image at the previous index has not been loaded then exit this function
                                                    }
                                                    productImageFileDialog.open()
                                                }
                                            }
                                            FileDialog {
                                                id: productImageFileDialog
                                                fileMode: FileDialog.OpenFile
                                                folder: (isWindows) ? StandardPaths.writableLocation(StandardPaths.DocumentsLocation) + "/neroshop" : StandardPaths.writableLocation(StandardPaths.HomeLocation) + "/neroshop"
                                                nameFilters: ["Image files (*.bmp *.gif *.jpeg *.jpg *.png *.tif *.tiff)"]
                                                onAccepted: productImageRepeater.itemAt(index).children[0].children[0].source = currentFile
                                            }
                                        }         
                                    }                       
                                } // flow
                            }
                        }
                    }
            // productMessageArea
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: childrenRect.width
                Layout.preferredHeight: childrenRect.height
                visible: (productPriceField.text.length <= 0)
                
                TextArea {
                    id: productMessageArea
                    anchors.topMargin: 20
                    anchors.leftMargin: 20; anchors.rightMargin: 20
                    width: 500
                    height: contentHeight + 20
                    selectByMouse: true
                    readOnly: true
                    verticalAlignment: TextEdit.AlignVCenter
                    wrapMode: TextEdit.Wrap
                    text: qsTr("Price field is empty")
                    color: (messageCode == 1) ? "#ffd700" : ((NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#404040")
                    property int messageCode: 1 // 0 = info; 1 = warning
                    background: Rectangle { 
                        color: "transparent"
                        border.color: (parent.messageCode == 1) ? "#ffd700" : "#2196f3"
                        radius: 3
                    }            
                    leftPadding: 30 + warningSign.contentWidth
                    Text {
                        id: warningSign
                        anchors.left: parent.left
                        anchors.leftMargin: 15
                        anchors.verticalCenter: parent.verticalCenter                         
                        text: (parent.messageCode == 1) ? qsTr(FontAwesome.circleExclamation) : qsTr(FontAwesome.circleInfo)
                        color: (parent.messageCode == 1) ? "#ffd700" : "#2196f3"
                        font.bold: true
                        font.family: FontAwesome.fontFamily
                    }
                }
            }        
                    // ListItem to "listings" table
                    Item {
                        Layout.alignment: Qt.AlignHCenter//Qt.AlignRight
                        Layout.preferredWidth: childrenRect.width//Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height
                        Row {
                            spacing: 5
                        // cancelButton
                        Button {
                            id: cancelButton
                            width: 166.666666667 - parent.spacing; height: contentItem.contentHeight + 30
                            hoverEnabled: true
                            text: qsTr("Cancel")
                            background: Rectangle {
                                color: parent.hovered ? "#b22222" : "#921c1c" 
                                radius: productDialog.inputRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                productDialog.close()
                                mainScrollView.ScrollBar.vertical.position = 0.0 // reset scrollbar
                            }
                        }    
                        // listButton
                        Button {
                            id: listProductButton
                            width: 333.333333333; height: contentItem.contentHeight + 30
                            hoverEnabled: true
                            text: qsTr("Submit")
                            background: Rectangle {
                                color: parent.hovered ? "#698b22" : "#506a1a"
                                radius: productDialog.inputRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                // Check input fields to see if entered info is valid
                                // ...
                                //---------------------------------------
                                if(productNameField.text.length < 3) {
                                    messageBox.text = qsTr("Item name is too short")
                                    messageBox.open()
                                    return; // exit function
                                }
                                //---------------------------------------
                                let subcategory_ids = []//let subcategories = []
                                for (let i = 0; i < subCategoryRepeater.count; i++) {
                                    subcategory_ids.push(Backend.getSubCategoryIdByName(subCategoryRepeater.itemAt(i).children[0].currentText))//subcategories.push(subCategoryRepeater.itemAt(i).children[0].currentText)//console.log("Added subcategory: ", subcategories[i])
                                }
                                // Product subcategories cannot be duplicated
                                if(subCategoryRepeater.count >= 2) {    
                                    if((new Set(subcategory_ids)).size !== subcategory_ids.length) {
                                        messageBox.text = "Item cannot have duplicate subcategories"
                                        messageBox.open()
                                        return; // exit function
                                    }
                                }
                                //---------------------------------------
                                // Product attributes (each attribute object represents a variant of the product)
                                let attributes = [];
                                let attribute_object = {};
                                if(productWeightField.text.length > 0 && Number(productWeightField.text) > 0.00) {
                                    if(weightMeasurementUnit.currentText !== "kg") {
                                        console.log("weight is in " + weightMeasurementUnit.currentText + ". Converting to kg ...")
                                        attribute_object.weight = Backend.weightToKg(Number(productWeightField.text), weightMeasurementUnit.currentText)
                                    } else {
                                        attribute_object.weight = Number(productWeightField.text)
                                    }
                                }
                                // Add attribute obj to list as long as its filled with properties
                                if (Object.keys(attribute_object).length > 0) {
                                    attributes.push(attribute_object)
                                }
                                //---------------------------------------
                                // Ship from location must be valid
                                if(productLocationBox.model.indexOf(productLocationBox.contentItem.text) === -1) {
                                    messageBox.text = "Invalid location entered"
                                    messageBox.open()
                                    return; // exit function
                                }
                                //---------------------------------------
                                // Product must have a minimum of 1 image
                                let productThumbnail = productImageRepeater.itemAt(0).children[0].children[0]
                                if(productThumbnail.status == Image.Null) {
                                    messageBox.text = "Item must have at least 1 image"
                                    messageBox.open()
                                    return; // exit function
                                }
                                //---------------------------------------
                                // One or more payment option must be selected
                                if(!paymentOptionsItem.isPaymentOptionSelected()) {
                                    messageBox.text = "At least one payment option must be selected"
                                    messageBox.open()
                                    return; // exit function
                                }
                                console.log("paymentOptionsItem.getSelectedPaymentOptions()",paymentOptionsItem.getSelectedPaymentOptions())
                                //---------------------------------------
                                // One or more payment coin must be selected
                                if(!paymentCoinsItem.isPaymentCoinSelected()) {
                                    messageBox.text = "No payment coin selected"
                                    messageBox.open()
                                    return; // exit function
                                }
                                console.log("paymentCoinsItem.getSelectedPaymentCoins()",paymentCoinsItem.getSelectedPaymentCoins())
                                //---------------------------------------
                                // One or more delivery option must be selected
                                if(!deliveryOptionsItem.isDeliveryOptionSelected()) {
                                    messageBox.text = "At least one delivery option must be selected"
                                    messageBox.open()
                                    return; // exit function
                                }
                                console.log("deliveryOptionsItem.getSelectedDeliveryOptions()",deliveryOptionsItem.getSelectedDeliveryOptions())
                                //---------------------------------------
                                // One or more shipping option must be selected
                                if(!shippingOptionsItem.isShippingOptionSelected() &&
                                    shippingOptionsItem.visible) {
                                    messageBox.text = "At least one shipping option must be selected"
                                    messageBox.open()
                                    return; // exit function
                                }
                                if(shippingOptionsItem.visible) console.log("shippingOptionsItem.getSelectedShippingOptions()",shippingOptionsItem.getSelectedShippingOptions())
                                //---------------------------------------
                                // Location must not be "Unspecified" if Pickup delivery option is selected
                                if(deliveryOptionsRepeater.itemAt(1).checked) {
                                    if(productLocationBox.contentItem.text == "Unspecified" ||
                                        productLocationBox.contentItem.text.length < 1) {
                                        messageBox.text = "Pickup location not specified"
                                        messageBox.open()
                                        return
                                    }
                                }
                                //---------------------------------------
                                // Create image objects with properties
                                let productImages = []
                                for(let i = 0; i < productImageRepeater.count; i++) {
                                    let productImage = productImageRepeater.itemAt(i).children[0].children[0]
                                    if(productImage.status == Image.Ready) { // If image loaded
                                        let image = Backend.uploadImageToObject(Backend.urlToLocalFile(productImage.source), i)
                                        if(Object.keys(image).length === 0) { return }
                                        if(!Backend.isSupportedImageDimension(image.width, image.height)) {
                                            messageBox.text = "Image dimensions must not exceed 1920x1280, 1280x1920, or 1600x1600"
                                            messageBox.open()
                                            return; // exit function
                                        }
                                        if(!Backend.isSupportedImageSizeBytes(image.size)) {
                                            messageBox.text = "Image size must not exceed 2 MB"
                                            messageBox.open()
                                            return; // exit function
                                        }
                                        productImages.push(image);
                                    }
                                }
                                //---------------------------------------
                                // List product
                                let listing_key = User.listProduct(
                                    productNameField.text, 
                                    productDescriptionEdit.text,
                                    attributes, 
                                    (productCodeField.text.length >= 6) ? productCodeType.currentText.toLowerCase() + ":" + productCodeField.text : "",
                                    Backend.getCategoryIdByName(productCategoryBox.currentText),
                                    (subCategoryRepeater.count > 0) ? subcategory_ids : [], // subcategoryIds
                                    productTagsField.tags(),
                                    productImages,
                                    
                                    productQuantityField.text, 
                                    productPriceField.text, 
                                    selectedCurrencyText.text, 
                                    productConditionBox.currentText, 
                                    productLocationBox.currentText,
                                    (quantityPerOrderField.text.length > 0) ? Number(quantityPerOrderField.text) : 0,
                                    paymentCoinsItem.getSelectedPaymentCoinsValue(),
                                    paymentOptionsItem.getSelectedPaymentOptionsValue(),
                                    deliveryOptionsItem.getSelectedDeliveryOptionsValue(),
                                    shippingOptionsItem.visible ? shippingOptionsItem.getSelectedShippingOptionsValue() : [],
                                    shippingCostsItem.getShippingCosts(),
                                    customRatesItem.getCustomRates()
                                )                       
                                // Save product thumbnail
                                Backend.saveProductThumbnail(productImages[0].source, listing_key)
                                // Save product image(s) to datastore folder
                                for (let i = 0; i < productImages.length; i++) {
                                    Backend.saveProductImage(productImages[i].source, listing_key)
                                }
                                // Clear input fields after listing product
                                productNameField.text = ""
                                productPriceField.text = ""
                                productQuantityField.text = "1"
                                productConditionBox.currentIndex = productConditionBox.find("New")
                                productCodeField.text = ""
                                productCategoryBox.currentIndex = productCategoryBox.find("Miscellaneous")
                                productCategoryBox.reset() // resets subcategories
                                productWeightField.text = ""
                                productLocationBox.currentIndex = productLocationBox.find("Unspecified")//find("Worldwide")
                                productDescriptionEdit.text = ""
                                productTagsField.clearTags()
                                quantityPerOrderField.text = ""
                                paymentOptionsItem.uncheckAllPaymentOptions()
                                ////paymentCoinsItem.uncheckAllPaymentCoins()
                                deliveryOptionsItem.uncheckAllDeliveryOptions()
                                shippingOptionsItem.uncheckAllShippingOptions()
                                // Clear upload images as well
                                for(let i = 0; i < productImageRepeater.count; i++) {
                                    let productImage = productImageRepeater.itemAt(i).children[0].children[0]
                                    productImage.source = ""
                                }
                                // close productDialog
                                productDialog.close()
                                // reset scrollbar
                                mainScrollView.ScrollBar.vertical.position = 0.0
                            }
                        }
                        }
                    }
        }
    }
}
