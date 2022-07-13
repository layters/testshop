#include "../include/carrier.hpp"

////////////////////
////////////////////
////////////////////
////////////////////
/*

    DB db("neroshop.db");
    db.table("carrier"); // checks if table exists first before creating it // id is primary key
    if(!db.table_exists("carrier")) { // add to table carrier if it does not exist
        db.column("carrier", "ADD", "name", "TEXT"); // carrier_name     (UPS, DHL, FedEX, USPS, etc.)
    }
    /////////////////////////////////
    db.table("mail"); // checks if table exists first before creating it
    if(!db.table_exists("mail")) { // add to table mail if it does not exist
        db.column("mail", "ADD", "carrier_id", "INTEGER"); // get from table carrier
        db.column("mail", "ADD", "tracking_number", "TEXT");
        db.index("idx_tracking_numbers", "mail", "tracking_number") // "tracking_number" must be unique
        db.column("mail", "ADD", "service", "TEXT"); // Standard, Express, 1 Day, 2 Day, etc.
        db.column("mail", "ADD", "shipping_cost", "INTEGER"); // shipping cost or rate
        
        db.column("mail", "ADD", "weight", "INTEGER"); // weight of letter or package
        db.column("mail", "ADD", "shipment_category", "TEXT"); // letter, package, etc.
        db.column("mail", "ADD", "shipping_date", "TEXT");
        //db.column("mail", "ADD", "", "TEXT");
    
    }    
*/
////////////////////
////////////////////
////////////////////
////////////////////
