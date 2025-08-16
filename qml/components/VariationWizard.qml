import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

//import FontAwesome 1.0

import "." as NeroshopComponents

Popup {
    id: variantPopup
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    width: Math.min(mainWindow.width * 0.95, 1000)  // 95% of main window width, max 1000px
    height: Math.min(mainWindow.height * 0.6, 400)  // 60% of height, max 400px
    visible: false
    modal: true // blocks input and interaction to items outside of itself
    focus: true // automatically grab keyboard input focus
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Background
    background: Rectangle {
        color: "white"
        radius: 8
        border.color: "#999"
        border.width: 1
    }

    property string baseProductCondition: ""
    property real baseProductWeight: 0.0
    property real baseProductPrice: 0.0 // <- must be set
    property int baseProductQuantity: 0 // <- must be set (maybe default should be 1?)
    
    property string productCodeType: ""
    property string weightUnit: ""
    
    property alias optionCombo: optionCombo
    
    property var variantModel: []
    
    function buildVariantsForCpp() {
        let variantsList = []

        for (let i = 0; i < variantModel.length; i++) {
            let v = variantModel[i]
            
            // Determine condition or fallback to placeholder
            let conditionStr = (v.condition !== undefined && v.condition !== null && v.condition.toString().trim().length > 0)
                ? v.condition.toString().trim()
                : baseProductCondition;

            // Determine weight or fallback to placeholder
            let weightStr = v.weight && v.weight.trim().length > 0 ? v.weight : baseProductWeight
            let weight = parseFloat(weightStr)
            if (isNaN(weight)) weight = 0.0
            // Convert weight based on weightUnit property
            if (weightUnit !== "kg") {
                console.log("weight is in " + weightUnit + ". Converting to kg ...")
                weight = Backend.weightToKg(weight, weightUnit).toFixed(3)
            }

            // Determine price or fallback
            let priceStr = v.price && v.price.trim().length > 0 ? v.price : baseProductPrice
            let price = parseFloat(priceStr)
            if (isNaN(price)) price = 0.0

            // Determine quantity or fallback
            let quantityStr = v.quantity && v.quantity.trim().length > 0 ? v.quantity : baseProductQuantity
            let quantity = parseInt(quantityStr)
            if (isNaN(quantity)) quantity = 0

            // Ensure imageIndex defaults to -1 if missing or invalid
            let imageIndex = (v.imageIndex !== undefined && v.imageIndex !== null && v.imageIndex.toString().trim().length > 0)
                         ? parseInt(v.imageIndex)
                         : -1;
            if (isNaN(imageIndex)) imageIndex = -1;

            // Build options map with the single option/value pair
            let options = {}
            if (v.option && v.value) {
                options[v.option.toLowerCase()] = v.value
            }

            variantsList.push({
                options: options,
                condition: conditionStr,
                weight: weight,
                price: price,
                quantity: quantity,
                product_code: v.productCode,
                image_index: imageIndex
            })
        }

        return variantsList
    }

    ColumnLayout {
        anchors.fill: parent
        ////anchors.margins: 20 // <- add this back if not using "scale" prop
        spacing: 12
        scale: 0.9 // scale entire content down to 90%
        transformOrigin: Item.Center // scale relative to center (optional but recommended)

        // Add attribute section
        RowLayout {
            spacing: 10

            ComboBox {
                id: optionCombo
                width: 120
                property string fixedOption: ""
                model: ["Color", "Size", "Material"]
                currentIndex: 0
                delegate: ItemDelegate {
                    text: modelData

                    // Enable only if either no fixedOption is set,
                    // or this option matches the fixedOption
                    enabled: (optionCombo.fixedOption === "") || (modelData === optionCombo.fixedOption)

                    // Optional: visually show disabled by adjusting color
                    // Not needed if default style already greys out disabled items
                    // color: enabled ? "black" : "gray"
                }
            }

            TextField {
                id: valuesField
                placeholderText: "Enter values, separated by commas"
                Layout.fillWidth: true
                selectByMouse: true
            }

            Button {
                text: "Add Variants"
                onClicked: {
                    let option = optionCombo.currentText;
                    let values = valuesField.text.split(",").map(function(v) { return v.trim(); }).filter(function(v) { return v.length > 0 });

                    for (let i = 0; i < values.length; i++) {
                        variantModel.push({
                            option: option,
                            value: values[i],
                            condition: "",
                            weight: "",
                            price: "",
                            quantity: "",
                            productCode: "",
                            imageIndex: ""
                        })
                    }
                    valuesField.text = ""
                    variantListView.model = variantModel//.slice()
                    
                    if(variantModel.length == 1 && values.length != 0) {
                        optionCombo.fixedOption = option
                        console.log("Fixed option set:",optionCombo.fixedOption)
                    }
                }
            }
        }

        // Table Header - start of wizard
        // Fixed header row (not scrollable)
        Item {
            id: headerRow
            width: parent.width  // or a fixed width if needed
            height: 30//40  // fixed header height
            
            Rectangle { 
                anchors.fill: parent
                color: "#e0e0e0"
                z: -1 // put behind the labels
                
                Label { id: optionHeader;    text: "Option";       x: 0;   width: 100; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: valueHeader;     text: "Value";        x: 100; width: 120; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: conditionHeader; text: "Condition";    x: 220; width: 120; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: weightHeader;    text: "Weight";       x: 340; width: 80;  font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: priceHeader;     text: "Price";        x: 420; width: 120; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: qtyHeader;       text: "Quantity";     x: 540; width: 80;  font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: prodCodeHeader;  text: "Product Code"; x: 620; width: 120; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: imageIdHeader;   text: "Image Index";  x: 740; width: 90;  font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                Label { id: actionsHeader;   text: "Actions";      x: 830; width: 80;  font.bold: true; anchors.verticalCenter: parent.verticalCenter }
            }
        }

        // Variants List
        ListView {
            id: variantListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: variantModel
            clip: true // makes sure contents don't overflow
            delegate: Item {
                id: delegateRoot
                property var popup: variantPopup
                width: variantListView.width//parent.width
                height: 40

                // Option Display (fixed, no combobox here)
                Label {
                    text: modelData.option
                    x: optionHeader.x
                    width: optionHeader.width//100
                    verticalAlignment: Text.AlignVCenter
                }

                // Value (editable TextField)
                TextField {
                    text: modelData.value
                    x: valueHeader.x
                    width: valueHeader.width//120
                    verticalAlignment: Text.AlignVCenter
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].value = text
                    }
                    ////readOnly:true
                }
                
                // Condition
                TextField {
                    text: modelData.condition
                    x: conditionHeader.x
                    width: conditionHeader.width
                    placeholderText: delegateRoot.popup.baseProductCondition
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].condition = text
                    }
                }

                // Weight
                TextField {
                    text: modelData.weight
                    x: weightHeader.x
                    width: weightHeader.width//80
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: RegExpValidator{ regExp: new RegExp("^-?[0-9]+(\\.[0-9]{1," + 3 + "})?$") }
                    placeholderText: delegateRoot.popup.baseProductWeight
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].weight = text
                    }
                }

                // Price
                TextField {
                    text: modelData.price
                    x: priceHeader.x
                    width: priceHeader.width//80
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    placeholderText: delegateRoot.popup.baseProductPrice
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].price = text
                    }
                }

                // Quantity
                TextField {
                    text: modelData.quantity
                    x: qtyHeader.x
                    width: qtyHeader.width//80
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: RegExpValidator{ regExp: /[0-9]*/ }
                    maximumLength: 9
                    placeholderText: delegateRoot.popup.baseProductQuantity
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].quantity = text
                    }
                }

                // Product Code
                TextField {
                    text: modelData.productCode
                    x: prodCodeHeader.x
                    width: prodCodeHeader.width//120
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].productCode = text
                    }
                }

                // Parent Image Index
                TextField {
                    text: modelData.imageIndex
                    x: imageIdHeader.x
                    width: imageIdHeader.width//90
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: RegExpValidator{ regExp: /[0-9]*/ }
                    onTextChanged: {
                        delegateRoot.popup.variantModel[index].imageIndex = text
                    }
                }

                // Remove button
                Button {
                    text: "Remove"
                    x: actionsHeader.x
                    width: actionsHeader.width//70
                    onClicked: {
                        delegateRoot.popup.variantModel.splice(index, 1)
                        variantListView.model = delegateRoot.popup.variantModel.slice()

                        if (delegateRoot.popup.variantModel.length > 0) {
                            delegateRoot.popup.optionCombo.fixedOption = delegateRoot.popup.variantModel[0].option
                        } else {
                            delegateRoot.popup.optionCombo.fixedOption = ""
                        }
                    }
                }
            }
        }
    }
}

