#include "user_controller.hpp"

neroshop::UserController::UserController() {
    // Should user not be initialized until they are logged in or?
    ////user = std::make_unique<neroshop::Seller>();
}

neroshop::UserController::~UserController() {
    if(user.get()) {
        user.reset();
    }
    std::cout << "user controller deleted\n";
}

void neroshop::UserController::listProduct(const QString& product_id, int quantity, double price, const QString& currency, const QString& condition, const QString& location) {
    if(!user.get()) throw std::runtime_error("neroshop::User is not initialized");
    static_cast<neroshop::Seller *>(user.get())->list_item(product_id.toStdString(), quantity, price, currency.toStdString(), condition.toStdString(), location.toStdString());
    emit productsCountChanged(); // TODO: emit this when delisting a product as well
}

void neroshop::UserController::uploadAvatar(const QString& filename) {
    if(!user.get()) throw std::runtime_error("neroshop::User is not initialized");
    user->upload_avatar(filename.toStdString());
}

bool neroshop::UserController::exportAvatar() {
    if(!user.get()) throw std::runtime_error("neroshop::User is not initialized");
    return user->export_avatar();
}
    
neroshop::User * neroshop::UserController::getUser() const {
    return user.get();
}

neroshop::Seller * neroshop::UserController::getSeller() const {
    return static_cast<neroshop::Seller *>(user.get());
}

QString neroshop::UserController::getID() const {
    if(!user.get()) throw std::runtime_error("neroshop::User is not initialized");
    return QString::fromStdString(user->get_id());
}

int neroshop::UserController::getProductsCount() const {
    return static_cast<neroshop::Seller *>(user.get())->get_products_count();
}

// to allow seller to use user functions: dynamic_cast<Seller *>(user)
