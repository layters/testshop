#ifndef CARRIER_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define CARRIER_HPP_NEROSHOP

#include <iostream>
#include "db.hpp"

namespace neroshop {
class Carrier {
public:
    static void track(const std::string& tracking_number); // opens browser and tracks an item
private:
};
}
#endif
/*
carrier (or courier)(table)
id - primary key
carrier_name     (UPS, DHL, FedEX, USPS, etc.)


parcel (or mail, or package)(table)
carrier_id (from table carrier)
tracking_number (must be unique)
shipping_service (Standard, Express, 1 Day, 2 Day, etc.)
shipping cost / rate - get from api
shipment_category (package, letter, etc.)
shipping_date
weight (weight of package, letter, etc.)
reference (or ref)(reference number or code)
...
address_to
address_from

to-do:
shipping address validator
open tracking url in browser


apis:
https://www.usps.com/business/web-tools-apis/
https://www.ups.com/upsdeveloperkit?loc=en_US
https://developer.fedex.com/api/en-us/home.html
https://developer.dhl.com/
*/
