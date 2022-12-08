import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls 1.4 as OC

	OC.TableView {
    		id: cartTable
    		width: parent.width
    		frameVisible: false
    		property int textPixelSize: 15
        	property int subHeaderPixelSize: 18
    		property string currencyRepresentation: '$'
    		function formatPriceRepresentation(price) {
    			return currencyRepresentation + parseFloat(price.toString(
    														   )).toFixed(2)
    		}
    		property var tableItemsModel: [{
    				"quantity": 2,
    				"price": 44,
    				"total": 88,
    				"product_details": {
    					"label": "Fifa 19",
    					"platform": "PS 4",
    					"photo": "qrc:/images/examples/product_1.jpg"
    				}
    			}, {
    				"quantity": 1,
    				"price": 44,
    				"total": 88,
    				"product_details": {
    					"label": "Glacier White 500GB",
    					"platform": "PS 4",
    					"photo": "qrc:/images/examples/product_2.jpg"
    				}
    			}, {
    				"quantity": 1,
    				"price": 249,
    				"total": 249.,
    				"product_details": {
    					"label": "Platinum Headset",
    					"platform": "PS 4",
    					"photo": "qrc:/images/examples/product_3.jpg"
    				}
    			}]
    		Component {
    			id: quantityButton
    			Button {
    				id: controlBt2
    				font.pixelSize: 25
    				implicitWidth: 30
    				implicitHeight: 30
    				text: isIncreaseButton ? '+' : '-'
    				background: Rectangle {
    					width: 30
    					height: 30
    					color: 'black'
    					opacity: controlBt2.down ? 0.5 : (controlBt2.hovered ? 0.1 : 0)
    					radius: 100
    				}
    			}
    		}
    		headerDelegate: Item {
    			height: 75
    			Rectangle {
    				height: childrenRect.height
    				width: 200
    				Text {
    					text: styleData.value
    					color: "#292933"
    					font.pixelSize: textPixelSize
    					anchors.horizontalCenter: styleData.column === 1 ? parent.horizontalCenter : parent.horizontalLeft
    				}
    			}
    		}
    		model: tableItemsModel
    		OC.TableViewColumn {
    			title: "PRODUCT DETAILS"
    			role: "quantity" // Bogus value, does nothing but suppressing warning
    			width: 450
    			resizable: false
    			delegate: Row {
    				id: row
    				spacing: 20
    				Column {
    					Image {
    						height: row.height - 50
    						width: 100
    						source: modelData.product_details.photo
    					}
    				}
    				Column {
    					height: row.height - 50
    					topPadding: detailsText.height / 2
    					spacing: (row.height / 3) - row.spacing - detailsText.height
    					Text {
    						id: detailsText
    						text: modelData.product_details.label
    						font.pixelSize: subHeaderPixelSize
    					}
    					Text {
    						text: modelData.product_details.platform
                            font.pixelSize: subHeaderPixelSize
    					}
    					Text {
                            id: removeButton
                            text: 'Remove'
                            color: 'gray'
                            font.pixelSize: subHeaderPixelSize
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    console.log("Implement delete item from model")
                                }
                                onEntered: {
                                    removeButton.color = 'black'
                                }
                                onExited: {
                                    removeButton.color = 'gray'
                                }
                            }
                        }
    				}
    			}
    		}

    		OC.TableViewColumn {
    			role: "quantity"
    			title: "QUANTITY"
    			width: 200
    			resizable: false
    			delegate: Item {
    				width: 200

    				Row {
    					anchors.horizontalCenter: parent.horizontalCenter

    					spacing: 5
    					Loader {
    						property bool isIncreaseButton: false
    						sourceComponent: quantityButton
    					}
    					TextField {
    						readOnly: true
    						width: 30
    						height: 30
    						text: '0'
    						horizontalAlignment: Text.AlignHCenter
    					}
    					Loader {
    						property bool isIncreaseButton: true
    						sourceComponent: quantityButton
    					}
    				}
    			}
    		}
    		OC.TableViewColumn {
    			role: "price"
    			title: "PRICE"
    			width: 200
    			resizable: false
    			delegate: Text {
    				text: formatPriceRepresentation(styleData.value)
    				font.pixelSize: subHeaderPixelSize
    			}
    		}
    		OC.TableViewColumn {
    			role: "total"
    			title: "TOTAL"
    			width: 200
    			resizable: false
    			delegate: Text {
    				text: formatPriceRepresentation(styleData.value)
    				font.pixelSize: subHeaderPixelSize
    			}
    		}
    		rowDelegate: Rectangle {
    			id: tableViewDelegate
    			height: 150
    		}
    	}