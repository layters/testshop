import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12 // ColorOverlay
import Qt.labs.platform 1.1 // FileDialog

import FontAwesome 1.0

import "../../components" as NeroshopComponents
// TODO: rename this file to Dashboard.qml?
Page {
    id: sellerHub
    background: Rectangle {
        color: "transparent"
    }
        
    NeroshopComponents.TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.topMargin: tabBar.buttonHeight
        anchors.horizontalCenter: parent.horizontalCenter
        model: ["Overview", "Inventory", "Customers"]
        Component.onCompleted: {
            buttonAt(1).checked = true //TODO: make "Inventory" default tab for quicker access to listing products
        }
    }
            
    StackLayout { // TODO: stackview (dashboard) with sections: overview (stats and analytics), inventory management (listing and delisting products), customer order management, reports, etc.
        id: dashboard
        width: parent.width; height: parent.height//2000//anchors.fill: parent
        anchors.top: tabBar.bottom; anchors.topMargin: tabBar.buttonHeight + 10//anchors.margins: 20
        currentIndex: tabBar.currentIndex
            
        Item {
            id: overviewTab
                // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true
            ScrollView {
                id: overviewTabScrollable
                anchors.fill: parent
                contentWidth: parent.childrenRect.width//parent.width
                contentHeight: parent.childrenRect.height + 100//* 2
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn////AsNeeded
                clip: true
                                
                RowLayout {
                    id: stats
                    anchors.horizontalCenter: parent.horizontalCenter//Layout.alignment: Qt.AlignHCenter | Qt.AlignTop// <- this is not working :/
                    property real numberTextFontSize: 24
                    property string textColor: "#384364"////(NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#384364"
                    property real boxWidth: 250//(scrollView.width / 3) - 20//scrollView.width / statsRepeater.count
                    property real boxHeight: 110
                    property real boxRadius: 6
                    spacing: 15
                    property bool useDefaultBoxColor: true//false
                    property string boxColor: "#ffffff"////(NeroshopComponents.Style.darkTheme) ? "#384364" : "#ffffff"
                    // Products (listed)
                    Rectangle {
                        Layout.preferredWidth: stats.boxWidth
                        Layout.preferredHeight: stats.boxHeight
                        color: stats.useDefaultBoxColor ? stats.boxColor : "#e9eefc"
                        radius: stats.boxRadius
                    
                        Item {
                            anchors.fill: parent
                            anchors.centerIn: parent
                        
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left; anchors.leftMargin: width / 2
                                width: 64; height: 64
                                color: "#e9eefc"
                                radius: 50
                                Image {
                                    id: productIcon
                                    source: "qrc:/images/open_parcel.png"
                                    width: 32; height: 32
                                    anchors.centerIn: parent
                                }
                            
                                ColorOverlay {
                                    anchors.fill: productIcon
                                    source: productIcon
                                    color: "#4169e1"
                                    visible: productIcon.visible
                                }
                            }
                            
                            Item {
                                anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                anchors.top: parent.children[0].top
                            
                                Text {
                                    text: "0"//"5430"
                                    font.bold: true
                                    font.pointSize: stats.numberTextFontSize
                                    color: stats.textColor
                                }
                    
                                Text {
                                    text: "Products"
                                    //font.bold: true
                                    color: stats.textColor
                                    anchors.left: parent.children[0].left
                                    anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                }
                            }
                        }
                    }
                    // Sales (the total number of completed orders)
                    Rectangle {
                        Layout.preferredWidth: stats.boxWidth
                        Layout.preferredHeight: stats.boxHeight
                        color: stats.useDefaultBoxColor ? stats.boxColor : "#eff5ef"
                        radius: stats.boxRadius
                    
                        Item {
                            anchors.fill: parent
                            anchors.centerIn: parent
                        
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left; anchors.leftMargin: width / 2
                                width: 64; height: 64
                                color: "#eff5ef"
                                radius: 50
                                Image {
                                    id: salesIcon
                                    source: "qrc:/images/increase.png"
                                    width: 32; height: 32
                                    anchors.centerIn: parent
                                }
                            
                                ColorOverlay {
                                    anchors.fill: salesIcon
                                    source: salesIcon
                                    color: "#8fbc8f"
                                    visible: salesIcon.visible
                                }
                            }
                        
                            Item {
                                anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                anchors.top: parent.children[0].top
                            
                                Text {
                                    text: "0"//"17440"
                                    font.bold: true
                                    font.pointSize: stats.numberTextFontSize
                                    color: stats.textColor
                                }
                    
                                Text {
                                    text: "Sales"
                                    //font.bold: true
                                    color: stats.textColor
                                    anchors.left: parent.children[0].left
                                    anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                }
                            }
                        }
                    }
                    // Ratings/Feedback/Reputation
                    Rectangle {
                        Layout.preferredWidth: stats.boxWidth
                        Layout.preferredHeight: stats.boxHeight
                        color: stats.useDefaultBoxColor ? stats.boxColor : "#fffbe5"
                        radius: stats.boxRadius
                    
                        Item {
                            anchors.fill: parent
                            anchors.centerIn: parent
                        
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left; anchors.leftMargin: width / 2
                                width: 64; height: 64
                                color: "#fffbe5"
                                radius: 50
                                Image {
                                    id: ratingIcon
                                    source: "qrc:/images/rating.png"
                                    width: 32; height: 32
                                    anchors.centerIn: parent
                                }
                            
                                ColorOverlay {
                                    anchors.fill: ratingIcon
                                    source: ratingIcon
                                    color: "#ffd700"//"#e6c200"
                                    visible: ratingIcon.visible
                                }
                            }
                        
                            Item {
                                anchors.right: parent.children[0].right; anchors.rightMargin: -(contentWidth + 20)
                                anchors.top: parent.children[0].top
                            
                                Text {
                                    text: qsTr("%1%2").arg("0").arg("%")
                                    font.bold: true
                                    font.pointSize: stats.numberTextFontSize
                                    color: stats.textColor
                                }
                    
                                Text {
                                    text: "Reputation"
                                    //font.bold: true
                                    color: stats.textColor
                                    anchors.left: parent.children[0].left
                                    anchors.top: parent.children[0].bottom; anchors.topMargin: 10
                                }
                            }
                        }
                    }
                } 
                // TODO: show recent orders from customers, show seller's top-selling products, show customer reviews on seller products
                //ColumnLayout {}
            } // ScrollView 
        } // overview tab
            
        Item {
            id: inventoryTab
            // StackLayout child Items' Layout.fillWidth and Layout.fillHeight properties default to true

            ScrollView {
                id: inventoryTabScrollable
                anchors.fill: parent//anchors.margins: 20
                contentWidth: parent.childrenRect.width//parent.width
                contentHeight: parent.childrenRect.height * 3//parent.height
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn////AsNeeded
                clip: true
                
                ColumnLayout {//GridLayout {//
                    id: productDetails
                    width: parent.width; height: childrenRect.height////anchors.fill: parent
                    spacing: 30////rowSpacing: 5
                    property string titleTextColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    property real radius: 10//3
                    property real spaceFromTitle: 10
                    property string textColor: (NeroshopComponents.Style.darkTheme) ? "#ffffff" : "#000000"
                    property string baseColor: (NeroshopComponents.Style.darkTheme) ? "#101010" : "#f0f0f0"
                    property string borderColor: (NeroshopComponents.Style.darkTheme) ? "#404040" : "#4d4d4d"
                    property string optTextColor: "#708090"
                    // RegisterItem to "products" table
                    //Product title (TODO: place both text and textfield in an item)
                    // If item fields are related then place them side-by-side in separate columns
                    Item {
                        //Layout.row: 0
                        Layout.alignment: Qt.AlignHCenter// | Qt.AlignTop
                        //Layout.topMargin: 0
                        Layout.preferredWidth: childrenRect.width////productNameField.width
                        Layout.preferredHeight: childrenRect.height // 72 (child margins included)

                        Column {//ColumnLayout {
                            //anchors.fill: parent // ?
                            spacing: productDetails.spaceFromTitle                        
                            Text {
                                text: "Product name" // height=17
                                color: productDetails.titleTextColor
                                font.bold: true
                            }
                            
                            TextField {
                                id: productNameField
                                width: 500; height: 50//Layout.preferredWidth: 500; Layout.preferredHeight: 50
                                placeholderText: qsTr("Product title")
                                color: productDetails.textColor
                                selectByMouse: true
                                maximumLength: 200
                                background: Rectangle { 
                                    color: productDetails.baseColor
                                    border.color: productDetails.borderColor
                                    border.width: parent.activeFocus ? 2 : 1
                                    radius: productDetails.radius
                                }
                            }
                        }
                    }
                    // Product price (sales price)
                    Item {
                        //Layout.row: 1
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {//ColumnLayout {
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Price"
                                color: productDetails.titleTextColor
                                font.bold: true
                            }                        
                        
                            Row {
                                spacing: 5//2
                                TextField {
                                    id: productPriceField
                                    width: 500/* - parent.children[1].width - parent.spacing*/; height: 50//Layout.preferredWidth: 500 - parent.children[1].width - parent.spacing; Layout.preferredHeight: 50
                                    placeholderText: qsTr("Enter price")
                                    color: productDetails.textColor
                                    selectByMouse: true
                                    validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + Backend.getCurrencyDecimals(settingsDialog.currency.currentText) + "})?$") }
                                    background: Rectangle { 
                                        color: productDetails.baseColor
                                        border.color: productDetails.borderColor
                                        border.width: parent.activeFocus ? 2 : 1
                                        radius: productDetails.radius
                                    }
                                    rightPadding: 25 + selectedCurrencyText.width
                                    function adjustPriceDecimals() {
                                        productPriceField.text = Number(productPriceField.text).toFixed(Backend.getCurrencyDecimals(settingsDialog.currency.currentText))
                                    }
                                    onEditingFinished: adjustPriceDecimals() // does not update when switching from crypto to fiat :(
                                    
                                    Text {
                                        id: selectedCurrencyText
                                        text: settingsDialog.currency.currentText
                                        color: productDetails.textColor
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
                                /*ComboBox {
                                    width: 100; height: parent.children[0].height//Layout.preferredWidth: 60; Layout.preferredHeight: parent.children[0].height
                                    //radius: parent.children[0].radius // <- oops ... ComboBoxes don't have a radius
                                    model: Backend.getCurrencyList()
                                }*/
                            }
                        }
                    }
                    // Product quantity
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Quantity"
                                color: productDetails.titleTextColor
                                font.bold: true
                                //visible: false
                            }
                            
                            TextField {
                                id: productQuantityField
                                width: 500; height: 50
                                placeholderText: qsTr("Enter quantity")
                                color: productDetails.textColor
                                selectByMouse: true
                                inputMethodHints: Qt.ImhDigitsOnly // for Android and iOS - typically used for input of languages such as Chinese or Japanese
                                validator: RegExpValidator{ regExp: /[0-9]*/ }
                                text: "1"
                                background: Rectangle { 
                                    color: productDetails.baseColor
                                    border.color: productDetails.borderColor
                                    border.width: parent.activeFocus ? 2 : 1
                                    radius: productDetails.radius
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
                                    color: productDetails.textColor
                                    font.bold: true////; font.family: FontAwesome.fontFamily//fontFamilySolid
                                    anchors.right: parent.right
                                    anchors.rightMargin: 15
                                    anchors.top: parent.top; anchors.topMargin: 5
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
                                    color: productDetails.textColor
                                    font.bold: true////; font.family: FontAwesome.fontFamily//fontFamilySolid
                                    anchors.right: parent.right
                                    anchors.rightMargin: 15
                                    anchors.bottom: parent.bottom; anchors.bottomMargin: 5
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
                        }
                    }                    
                    // Product condition
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Condition"
                                color: productDetails.titleTextColor
                                font.bold: true
                                //visible: false
                            }
                            NeroshopComponents.ComboBox {
                                id: productConditionBox
                                width: 500; height: 50
                                model: ["New", "Used", "Refurbished (Renewed)"] // default is New
                                Component.onCompleted: currentIndex = find("New")
                                radius: productDetails.radius
                                color: productDetails.baseColor
                                textColor: productDetails.textColor
                            }
                        }
                    }                    
                    // Product code UPC, EAN, JAN, SKU, ISBN (for books) // https://www.simplybarcodes.com/barcode_types.html
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDetails.spaceFromTitle
                            
                            Row {
                                spacing: 10
                                Text {
                                    text: "Product code"
                                    color: productDetails.titleTextColor
                                    font.bold: true
                                }
                                Text {
                                    text: "(OPTIONAL)"
                                    color: productDetails.optTextColor
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
                                    color: productDetails.textColor
                                    selectByMouse: true
                                    background: Rectangle { 
                                        color: productDetails.baseColor
                                        border.color: productDetails.borderColor
                                        border.width: parent.activeFocus ? 2 : 1
                                        radius: productDetails.radius
                                    }                            
                                }
                                NeroshopComponents.ComboBox {
                                    id: productCodeType
                                    height: parent.children[0].height//Layout.preferredWidth: 100; Layout.preferredHeight: parent.children[0].height
                                    //radius: parent.children[0].radius // <- oops ... ComboBoxes don't have a radius
                                    model: ["EAN", "ISBN", "JAN", "SKU", "UPC"] // default is UPC (each code will be validated before product is listed)
                                    Component.onCompleted: currentIndex = find("UPC")
                                    radius: productDetails.radius
                                    color: productDetails.baseColor
                                    textColor: productDetails.textColor
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
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Category"
                                color: productDetails.titleTextColor
                                font.bold: true
                            }
                            
                            Row {
                                spacing: 5
                                NeroshopComponents.ComboBox {
                                    id: productCategoryBox
                                    width: 500; height: 50
                                    model: parent.parent.parent.getCategoryStringList()
                                    Component.onCompleted: {
                                        currentIndex = find("Miscellaneous")
                                    }
                                    radius: productDetails.radius
                                    color: productDetails.baseColor
                                    textColor: productDetails.textColor
                                }
                            }
                        }
                    }                    
                    // Subcategories (will be determined based on selected categories)
                    // Weight
                    Item {
                        //Layout.row: 
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: childrenRect.width
                        Layout.preferredHeight: childrenRect.height
                        
                        Column {
                            spacing: productDetails.spaceFromTitle
                            Row {
                                spacing: 10
                                Text {
                                    text: "Weight"
                                    color: productDetails.titleTextColor
                                    font.bold: true
                                }
                                Text {
                                    text: "(OPTIONAL)"
                                    color: productDetails.optTextColor
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
                                    color: productDetails.textColor
                                    selectByMouse: true
                                    validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + 8 + "})?$") }
                                    background: Rectangle { 
                                        color: productDetails.baseColor
                                        border.color: productDetails.borderColor
                                        border.width: parent.activeFocus ? 2 : 1
                                        radius: productDetails.radius
                                    }
                                }
                                NeroshopComponents.ComboBox {
                                    id: weightMeasurementUnit
                                    height: parent.children[0].height
                                    model: ["kg", "lb"] // default is kg (every unit of measurement will be converted to kg)
                                    Component.onCompleted: currentIndex = find("kg")
                                    radius: productDetails.radius
                                    color: productDetails.baseColor
                                    textColor: productDetails.textColor
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
                            spacing: productDetails.spaceFromTitle
                            Row {
                                spacing: 10
                                Text {
                                    text: "Size"
                                    color: productDetails.titleTextColor
                                    font.bold: true
                                }
                                Text {
                                    text: "(OPTIONAL)"
                                    color: productDetails.optTextColor
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
                                    color: productDetails.textColor
                                    selectByMouse: true
                                    background: Rectangle { 
                                        color: productDetails.baseColor
                                        border.color: productDetails.borderColor
                                        border.width: parent.activeFocus ? 2 : 1
                                        radius: productDetails.radius
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
                        property var countriesModel: ["Afghanistan", "Albania", "Algeria", "Andorra", "Angola", "Antigua & Deps", "Argentina", "Armenia", "Australia", "Austria", "Azerbaijan", "Bahamas", "Bahrain", "Bangladesh", "Barbados", "Belarus", "Belgium", "Belize", "Benin", "Bermuda", "Bhutan", "Bolivia", "Bosnia Herzegovina", "Botswana", "Brazil", "Brunei", "Bulgaria", "Burkina", "Burundi", "Cambodia", "Cameroon", "Canada", "Cape Verde", "Central African Rep", "Chad", "Chile", "China", "Colombia", "Comoros", "Congo", "Congo (Democratic Rep)", "Costa Rica", "Croatia", "Cuba", "Cyprus", "Czech Republic", "Denmark", "Djibouti", "Dominica", "Dominican Republic", "East Timor", "Ecuador", "Egypt", "El Salvador", "Equatorial Guinea", "Eritrea", "Estonia", "Eswatini", "Ethiopia", "Fiji", "Finland", "France", "Gabon", "Gambia", "Georgia", "Germany", "Ghana", "Greece", "Grenada", "Guatemala", "Guinea", "Guinea-Bissau", "Guyana", "Haiti", "Honduras", "Hungary", "Iceland", "India", "Indonesia", "Iran", "Iraq", "Ireland (Republic)", "Israel", "Italy", "Ivory Coast", "Jamaica", "Japan", "Jordan", "Kazakhstan", "Kenya", "Kiribati", "Korea North", "Korea South", "Kosovo", "Kuwait", "Kyrgyzstan", "Laos", "Latvia", "Lebanon", "Lesotho", "Liberia", "Libya", "Liechtenstein", "Lithuania", "Luxembourg", "Macedonia", "Madagascar", "Malawi", "Malaysia", "Maldives", "Mali", "Malta", "Marshall Islands", "Mauritania", "Mauritius", "Mexico", "Micronesia", "Moldova", "Monaco", "Mongolia", "Montenegro", "Morocco", "Mozambique", "Myanmar", "Namibia", "Nauru", "Nepal", "Netherlands", "New Zealand", "Nicaragua", "Niger", "Nigeria", "Norway", "Oman", "Pakistan", "Palau", "Palestine", "Panama", "Papua New Guinea", "Paraguay", "Peru", "Philippines", "Poland", "Portugal", "Qatar", "Romania", "Russian Federation", "Rwanda", "St Kitts & Nevis", "St Lucia", "Saint Vincent & the Grenadines", "Samoa", "San Marino", "Sao Tome & Principe", "Saudi Arabia", "Senegal", "Serbia", "Seychelles", "Sierra Leone", "Singapore", "Slovakia", "Slovenia", "Solomon Islands", "Somalia", "South Africa", "South Sudan", "Spain", "Sri Lanka", "Sudan", "Suriname", "Sweden", "Switzerland", "Syria", "Taiwan", "Tajikistan", "Tanzania", "Thailand", "Togo", "Tonga", "Trinidad & Tobago", "Tunisia", "Turkey", "Turkmenistan", "Tuvalu", "Uganda", "Ukraine", "United Arab Emirates", "United Kingdom", "United States", "Unspecified", "Uruguay", "Uzbekistan", "Vanuatu", "Vatican City", "Venezuela", "Vietnam", "Yemen", "Zambia", "Zimbabwe", "Worldwide",]
                        
                        Column {
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Location"
                                color: productDetails.titleTextColor
                                font.bold: true
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
                                radius: productDetails.radius
                                color: productDetails.baseColor
                                textColor: productDetails.textColor
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
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Description"
                                color: productDetails.titleTextColor
                                font.bold: true
                            }
                            
                            ScrollView {
                                width: 500; height: 250
                                TextArea {
                                    id: productDescriptionEdit
                                    placeholderText: qsTr("Enter description")
                                    wrapMode: Text.Wrap //Text.Wrap moves text to the newline when it reaches the width//Text.WordWrap does not move text to the newline but instead it only shows the scrollbar
                                    selectByMouse: true
                                    color: productDetails.textColor
                            
                                    background: Rectangle {
                                        color: productDetails.baseColor
                                        border.color: productDetails.borderColor
                                        border.width: parent.activeFocus ? 2 : 1
                                        radius: productDetails.radius
                                    }
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
                            spacing: productDetails.spaceFromTitle
                            Text {
                                text: "Upload image(s)"
                                color: productDetails.titleTextColor
                                font.bold: true
                            }
                            
                            Flickable {
                                width: 500; height: 210 + ScrollBar.horizontal.height// same height as delegateRect + scrollbar height
                                contentWidth: (210 * productImageRepeater.count) + (5 * (productImageRepeater.count - 1))//<- 5 is the Flow.spacing//; contentHeight: 210//<- contentHeight is not needed unless a newline is supported
                                clip: true
                                ScrollBar.horizontal: ScrollBar {
                                    policy: ScrollBar.AsNeeded
                                }
                                Flow {
                                    width: parent.contentWidth; height: parent.height//anchors.fill: parent// Note: Flow width must be large enough to fit all items horizontally so that there won't be a need to move an item to a newline
                                    spacing: 5
                                    Repeater {
                                        id: productImageRepeater
                                        model: 6
                                        delegate: Rectangle {//Item { 
                                            width: 210; height: 210
                                            color: "transparent"
                                            //border.color: "blue"
                                        
                                            Rectangle {
                                                border.color: productDetails.titleTextColor
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
                                                        }
                                                        if(this.status == Image.Loading) console.log("Loading image " + source + " (" + (progress * 100) + "%)")
                                                        if(this.status == Image.Error) console.log("An error occurred while loading the image")
                                                        //if(this.status == Image.Null) console.log("No image has been set" + parent.parent.index)
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
                                                    radius: productDetails.radius
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
                    // ListItem to "listings" table
                    Item {
                        Layout.alignment: Qt.AlignHCenter//Qt.AlignRight
                        Layout.preferredWidth: childrenRect.width//Layout.fillWidth: true
                        Layout.preferredHeight: childrenRect.height                    
                        // No need to add Column nor Row since we only have one Button in this Item
                        Button {
                            id: listProductButton
                            width: 500; height: contentItem.contentHeight + 30
                            text: qsTr("List Product")
                            background: Rectangle {
                                color: parent.hovered ? "#4d426c" : "#443a5f"
                                radius: productDetails.radius
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
                                // TODO: product must have a minimum of 1 image
                                let productThumbnail = productImageRepeater.itemAt(0).children[0].children[0]
                                if(productThumbnail.status == Image.Null) {
                                    messageBox.text = "Product must have at least 1 image"
                                    messageBox.open()
                                    return; // exit function
                                }
                                ////User.listProduct()//TODO:
                                // TODO: add weight and product_code to attributes list instead
                                // Attributes will be in JSON format
                                // Todo: check whether its a product or service
                                // Register product to database
                                let register_result = Backend.registerProduct(productNameField.text, productDescriptionEdit.text, 
                                    productWeightField.text, ""/*attributes*/, productCodeField.text,
                                    Backend.getCategoryIdByName(productCategoryBox.currentText))
                                // Upload product images to database
                                let product_uuid = register_result[1]                                
                                if(register_result[0] == true) {
                                    for(let i = 0; i < productImageRepeater.count; i++) {
                                        let productImage = productImageRepeater.itemAt(i).children[0].children[0]
                                        if(productImage.status == Image.Ready) { // If image loaded
                                            //console.log("uploading " + Backend.urlToLocalFile(productImage.source) + " to the database")
                                            Backend.uploadProductImage(product_uuid, Backend.urlToLocalFile(productImage.source))
                                        }
                                    }
                                }
                                // List product
                                User.listProduct(product_uuid, productQuantityField.text, productPriceField.text, selectedCurrencyText.text, productConditionBox.currentText, productLocationBox.currentText)                       
                                // Clear input fields after listing product
                                productNameField.text = ""
                                productPriceField.text = ""
                                productQuantityField.text = "1"
                                productConditionBox.currentIndex = productConditionBox.find("New")
                                productCodeField.text = ""
                                productCategoryBox.currentIndex = productCategoryBox.find("Miscellaneous")
                                productWeightField.text = ""
                                productLocationBox.currentIndex = productLocationBox.find("Unspecified")//find("Worldwide")
                                productDescriptionEdit.text = ""
                                // Clear upload images as well
                                for(let i = 0; i < productImageRepeater.count; i++) {
                                    let productImage = productImageRepeater.itemAt(i).children[0].children[0]
                                    productImage.source = ""
                                }
                            }
                        }
                    }
                }
                
                //ColumnLayout {
                //    id: inventoryManager // inventory can be managed here and sorted too
                //}    
            } // ScrollView
        } // inventoryTab
            
        Item {
            id: customerOrdersTab
            // TODO: order status: pending, unpaid, delivered/completed, refunded, etc.
            ScrollView {
                id: customerOrdersTabScrollable
                anchors.fill: parent//anchors.margins: 20
                contentWidth: parent.childrenRect.width//parent.width
                contentHeight: parent.childrenRect.height * 3//parent.height
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn////AsNeeded
                clip: true
            }            
        } 
    } // StackLayout
}
