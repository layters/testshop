#include "user_controller.hpp"

neroshop::UserController::UserController(QObject *parent) : QObject(parent)
{
    // Should user not be initialized until they are logged in or?
    ////user = std::make_unique<neroshop::Seller>();
}

neroshop::UserController::~UserController() {
    std::cout << "user controller deleted\n";
}

void neroshop::UserController::listProduct(const QString& product_id, int quantity, double price, const QString& currency, const QString& condition, const QString& location) {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    seller->list_item(product_id.toStdString(),
                      quantity,
                      price,
                      currency.toStdString(),
                      condition.toStdString(),
                      location.toStdString());
    emit productsCountChanged(); // TODO: emit this when delisting a product as well
}

void neroshop::UserController::uploadAvatar(const QString& filename) {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    _user->upload_avatar(filename.toStdString());
}

bool neroshop::UserController::exportAvatar() {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    return _user->export_avatar();
}
    
neroshop::User * neroshop::UserController::getUser() const {
    return _user.get();
}

neroshop::Seller * neroshop::UserController::getSeller() const {
    return static_cast<neroshop::Seller *>(_user.get());
}

QString neroshop::UserController::getID() const {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    return QString::fromStdString(_user->get_id());
}

int neroshop::UserController::getProductsCount() const {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    return dynamic_cast<neroshop::Seller *>(_user.get())->get_products_count();
}

// to allow seller to use user functions: dynamic_cast<Seller *>(user)
