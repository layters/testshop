#pragma once

#include <iostream>
#include <string>

namespace neroshop {

// delivery options
enum class DeliveryOption {
    Delivery = 0, // Ship
    Pickup, // Can include Dead drops and In-Store/Curbside pickups
};

// delivery methods (does not apply to Pickup delivery options)
enum class DeliveryMethod {
    Mail = 0,
    Courier,
    Digital,
};

// delivery package type (does not apply to Digital deliveries)
enum class PackageType {
    Parcel = 0, // Can include poly mailers, boxes, etc.
    Envelope,
    Box,
    Pallet,
};

enum class ShippingOption {
    Standard, // 3-7 business days depending on country, cost: $5.00
    Expedited, // (Priority) // 1-3 business days domestically, 3-5 days internationally, cost: $15.00
    Express, // 1-2 days domestically, 1-5 days internationally, cost: $25.00
    /*NextDay,
    Overnight = NextDay,*/
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

}
