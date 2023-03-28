// This is the order confirmation/checkout page that is displayed before an order is placed
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../components" as NeroshopComponents

Page {
    background: Rectangle {
        color: "transparent"
    }
    RowLayout {
	    width: parent.width
		Button {
			implicitWidth: parent.width
			implicitHeight: 50
			text: "Place order"
			font.pixelSize: 16
			background: Rectangle {
				color: "#8071a8"
			}
			onClicked: {
			    const shipping_obj = {
			        // name(first, last)
			        name: "Jack", // first, last
			        // address1(street, p.o box, company name, etc.)
			        address: "12 Robot Street",
			        // address2(apt number, suite, unit, building, floor, etc.)
		            apt_number: "",//// suite = apt_number, unit = apt_number, building = apt_number, floor = apt_number,
		            city: "Boston",
		            state: "Massachusetts",//// province: state, region: state,		            
		            zip_code: "02115",//// postal_code: zip_code,
		            // notes - should be placed outside shipping_obj
		            //notes: "" // any additional notes like delivery instructions or contact information
			    };
				const json = JSON.stringify(shipping_obj);
				User.createOrder(json)
			}
		}
	}
}
