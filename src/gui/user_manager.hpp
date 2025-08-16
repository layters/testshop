#pragma once

#ifndef USER_MANAGER_HPP_NEROSHOP
#define USER_MANAGER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QObject>
#include <QString>
#include <QVariant>
#endif
#include <memory> // std::unique_ptr

#include "wallet_manager.hpp"

#include "../core/market/seller.hpp"////"../core/market/user.hpp"

namespace neroshop {

class UserManager : public QObject, public neroshop::Seller {
    Q_OBJECT
public:
    UserManager(QObject *parent = nullptr);
    ~UserManager();

    Q_PROPERTY(neroshop::User* user READ getUser NOTIFY userChanged);
    Q_PROPERTY(bool logged READ isUserLogged NOTIFY userLogged);
    Q_PROPERTY(int productsCount READ getProductsCount NOTIFY productsCountChanged);
    Q_PROPERTY(int cartQuantity READ getCartQuantity NOTIFY cartQuantityChanged);
    Q_PROPERTY(int messagesCount READ getMessagesCount NOTIFY messagesCountChanged);

    Q_PROPERTY(QVariantList inventory READ getInventory NOTIFY inventoryChanged);
    //Q_PROPERTY(QVariantList cart READ getCart NOTIFY cartQuantityChanged);

    Q_INVOKABLE QString listProduct(
        const QString& name, 
        const QString& description,
        double weight,
        const QList<QVariantMap>& variants, 
        const QString& product_code,
        int category_id, 
        const QList<int>& subcategory_ids, 
        const QStringList& tags,
        const QList<QVariantMap>& images,
        
        int quantity, 
        double price, 
        const QString& currency, 
        const QString& condition, 
        const QString& location, 
        int quantity_per_order, 
        /*int payment_method,*/ 
        const QList<int>& payment_coins, 
        const QList<int>& payment_options, 
        const QList<int>& delivery_options,
        const QList<int>& shipping_options,
        const QList<QVariantMap>& shipping_costs,
        const QList<QVariantMap>& custom_rates
    );
    Q_INVOKABLE void delistProduct(const QString& listing_key);
    Q_INVOKABLE void delistProducts(const QStringList& listing_keys);
    Q_INVOKABLE int addToCart(const QString& listing_key, int quantity);
    //Q_INVOKABLE void removeFromCart(const QString& listing_key, int quantity);
    Q_INVOKABLE void createOrder(const QString& shipping_address);
    Q_INVOKABLE void rateItem(const QString& product_id, int stars, const QString& comments);//, const QString& signature);
    Q_INVOKABLE void rateSeller(const QString& seller_id, int score, const QString& comments);//, const QString& signature);
    
    Q_INVOKABLE void addToFavorites(const QString& listing_key);
    Q_INVOKABLE void removeFromFavorites(const QString& listing_key);
    Q_INVOKABLE bool hasFavorited(const QString& listing_key);
    
    //Q_INVOKABLE void exportCartData();
    //Q_INVOKABLE void exportFavoritesData();
    
    //Q_INVOKABLE void setID(const QString& id);
    //Q_INVOKABLE void setWallet(neroshop::WalletManager * wallet); // get the actual wallet from the controller then set it as the wallet
    Q_INVOKABLE void setStockQuantity(const QString& listing_key, int quantity);
    
    Q_INVOKABLE void uploadAvatar(const QString& filename);
    
    Q_INVOKABLE void sendMessage(const QString& recipient_id, const QString& content, const QString& recipient_public_key);
    Q_INVOKABLE QVariantMap decryptMessage(const QString& content_encoded, const QString& sender_encoded);
        
    Q_INVOKABLE QString getId() const;//Q_INVOKABLE neroshop::WalletManager * getWallet() const;
    Q_INVOKABLE int getProductsCount() const;
    Q_INVOKABLE int getReputation() const;
    //Q_INVOKABLE <type> <function_name>() const;
    Q_INVOKABLE int getCartQuantity() const;
    
    Q_INVOKABLE QVariantList getInventory(int sorting = 0) const;

    Q_INVOKABLE QVariantList getMessages() const;
    Q_INVOKABLE QVariantList getMessages(const QString& sender_id) const;
    Q_INVOKABLE QVariantList getMessages(const QString& sender_id, const QVariantList& messages) const;
    Q_INVOKABLE int getMessagesCount() const;

    Q_INVOKABLE neroshop::User * getUser() const;
    neroshop::Seller * getSeller() const;    
    
    Q_INVOKABLE bool isUserLogged() const;
    //Q_INVOKABLE void onLogin();
    friend class Backend;
signals:
    void userChanged();
    void userLogged();
    void inventoryChanged(); // for inventory sorting
    void productsCountChanged();
    void cartQuantityChanged();
    void messagesCountChanged();
private:
    std::unique_ptr<neroshop::User> _user;
    //std::unique_ptr<neroshop::WalletManager> wallet_manager;
};

}

#endif
