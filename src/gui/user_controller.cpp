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

void neroshop::UserController::uploadAvatar(const QString& filename) {
    if(!user.get()) throw std::runtime_error("neroshop::User is not initialized");
    user->upload_avatar(filename.toStdString());
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

// to allow seller to use user functions: dynamic_cast<Seller *>(user)
