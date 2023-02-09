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

void neroshop::UserController::rateItem(const QString& product_id, int stars, const QString& comments)
{
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());

    std::string signature = seller->get_wallet()->sign_message(comments.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    std::cout << "Signature: \033[33m" << signature << "\033[0m" << std::endl;
    std::string verified = (seller->get_wallet()->verify_message(comments.toStdString(), signature)) ? "true" : "false";
    std::cout << "Is verified: " << ((verified == "true") ? "\033[32m" : "\033[31m") << verified << "\033[0m" << std::endl;
    
    _user->rate_item(product_id.toStdString(), stars, comments.toStdString(), signature);//signature.toStdString());
}

void neroshop::UserController::rateSeller(const QString& seller_id, int score, const QString& comments)
{
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());

    std::string signature = seller->get_wallet()->sign_message(comments.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    std::cout << "Signature: \033[33m" << signature << "\033[0m" << std::endl;
    std::string verified = (seller->get_wallet()->verify_message(comments.toStdString(), signature)) ? "true" : "false";
    std::cout << "Is verified: " << ((verified == "true") ? "\033[32m" : "\033[31m") << verified << "\033[0m" << std::endl;
    
    _user->rate_seller(seller_id.toStdString(), score, comments.toStdString(), signature);//signature.toStdString());
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
