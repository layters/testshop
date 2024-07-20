#pragma once

#include <iostream>
#include <string>

namespace neroshop {

// delivery options/methods
enum class DeliveryOption {
    Shipping = 0,
    Pickup, // Can include Dead drops and In-Store/Curbside pickups
    Digital,
};

// package type
enum class PackageType {
    None = -1, // Intended for digital delivery options
    Parcel, // Can include poly mailers, boxes, etc.
    Envelope,
    Box,
    Pallet,
};

// shipping methods
enum class ShippingMethod {
    Mail = 0,
    Courier,
};

enum class ShippingOption {
    Standard = 0, // 3-7 business days depending on country, cost: $5.00
    Expedited, // (Priority) // 1-3 business days domestically, 3-5 days internationally, cost: $15.00
    Express, // 1-2 days domestically, 1-5 days internationally, cost: $25.00
    NextDay,
    Overnight = NextDay,
    SameDay,
    LocalDelivery = SameDay,
    International, // 1-4 weeks
    EcoFriendly,
};

enum class CourierService {
    DHL,
    FedEx,
    UPS,
    USPS,
    Aramex,
    TNT,
    CanadaPost,
    DBSchenker,
    Purolator,
    RLCarriers, // R+L Carriers
    YRCFreight,
    DTDC,
    BlueDart,
    PostNL,
    RoyalMail,
    AustraliaPost,
    JapanPost,
    DeutschePost,
    LaPoste,
    Correos,
    PosteItaliane,
};

//-----------------------------------------------------------------------------

inline std::string get_delivery_option_as_string(DeliveryOption delivery_option) {
    switch(delivery_option) {
        case DeliveryOption::Shipping: return "Shipping";
        case DeliveryOption::Pickup: return "Pickup";
        case DeliveryOption::Digital: return "Digital";
        default: return "";
    }
}

inline std::string get_shipping_option_as_string(ShippingOption shipping_option) {
    switch(shipping_option) {
        case ShippingOption::Standard: return "Standard";
        case ShippingOption::Expedited: return "Expedited";
        case ShippingOption::Express: return "Express";
        case ShippingOption::Overnight: return "Overnight"; // NextDay
        case ShippingOption::LocalDelivery: return "LocalDelivery"; // SameDay
        case ShippingOption::International: return "International";
        case ShippingOption::EcoFriendly: return "EcoFriendly";
        default: return "";
    }
}

}
