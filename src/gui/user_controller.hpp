#pragma once

#ifndef USER_CONTROLLER_HPP_NEROSHOP
#define USER_CONTROLLER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QObject>
#include <QString>
#include <QVariant>
#endif
#include <memory> // std::unique_ptr

#include "wallet_controller.hpp"

#include "../core/seller.hpp"////"../core/user.hpp"

namespace neroshop {

class UserController : public QObject, public neroshop::Seller/*User*/ {
    Q_OBJECT
public:
    UserController(QObject *parent = nullptr);
    ~UserController();
    
    Q_PROPERTY(neroshop::User* user READ getUser);// NOTIFY ?);
    Q_PROPERTY(int productsCount READ getProductsCount NOTIFY productsCountChanged);

    Q_INVOKABLE void listProduct(const QString& product_id, int quantity, double price, const QString& currency, const QString& condition, const QString& location);
    //Q_INVOKABLE void addToCart();
    //Q_INVOKABLE void createOrder();
    Q_INVOKABLE void rateItem(const QString& product_id, int stars, const QString& comments);//, const QString& signature);
    Q_INVOKABLE void rateSeller(const QString& seller_id, int score, const QString& comments);//, const QString& signature);
    //Q_INVOKABLE void addToFavorites();
    //Q_INVOKABLE void removeFromFavorites();
    
    //Q_INVOKABLE void setID(const QString& id);
    //Q_INVOKABLE void setWallet(neroshop::WalletController * wallet); // get the actual wallet from the controller then set it as the wallet
    
    Q_INVOKABLE void uploadAvatar(const QString& filename);
    Q_INVOKABLE bool exportAvatar();
        
    Q_INVOKABLE QString getID() const;
    //Q_INVOKABLE neroshop::WalletController * getWallet() const;
    Q_INVOKABLE int getProductsCount() const;
    //Q_INVOKABLE <type> <function_name>() const;

    Q_INVOKABLE neroshop::User * getUser() const;
    neroshop::Seller * getSeller() const;    
    
    //Q_INVOKABLE void onLogin();
    friend class Backend;
signals:
    void productsCountChanged();
private:
    std::unique_ptr<neroshop::User> _user;
    //std::unique_ptr<neroshop::WalletController> wallet_controller;
};

}

#endif
