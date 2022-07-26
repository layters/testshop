#include "../include/cart.hpp"

////////////////////
neroshop::Cart::Cart() : id(0), max_items(10), max_quantity(100) {} // cart can only hold up to 10 unique items // cart items can only add up to 100 qty
////////////////////
neroshop::Cart::~Cart() {
    contents.clear(); // this should reset (delete) all cart items
#ifdef NEROSHOP_DEBUG
    std::cout << "cart deleted\n";
#endif
}
////////////////////
////////////////////
////////////////////
// normal
////////////////////
void neroshop::Cart::add(unsigned int item_id, int quantity) {
}
////////////////////
void neroshop::Cart::add(const neroshop::Item& item, int quantity) {
    add(item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::remove(unsigned int item_id, int quantity) {
}
////////////////////
void neroshop::Cart::remove(const neroshop::Item& item, int quantity) {
    remove(item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::empty() {
}
////////////////////
void neroshop::Cart::change_quantity(const neroshop::Item& item, int quantity) {
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
unsigned int neroshop::Cart::get_id() const {
    return id;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::Cart::is_empty() const {
}
////////////////////
bool neroshop::Cart::is_full() const {
}
////////////////////
bool neroshop::Cart::in_cart(unsigned int item_id) const {
}
////////////////////
bool neroshop::Cart::in_cart(const neroshop::Item& item) const {
    return in_cart(item.get_id());
}
////////////////////
////////////////////
////////////////////
////////////////////
