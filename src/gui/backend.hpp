#pragma once

#ifndef BACKEND_HPP_NEROSHOP
#define BACKEND_HPP_NEROSHOP

#include <QObject>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QClipboard>
#include <QGuiApplication>

#include "../core/currency_converter.hpp"
#include "../core/validator.hpp"
#include "../core/seller.hpp"

#include "wallet_proxy.hpp"

#include <iostream>

namespace neroshop {
class Backend : public QObject { // This class was created for storing utility functions and backend implementations // Maybe I should rename this to BackendTools?
    Q_OBJECT 
public:
    Q_INVOKABLE QString urlToLocalFile(const QUrl& url) const;
    Q_INVOKABLE void copyTextToClipboard(const QString& text);
    
    Q_INVOKABLE QStringList getCurrencyList() const;
    Q_INVOKABLE int getCurrencyDecimals(const QString& currency) const;
    Q_INVOKABLE double getPrice(double amount, const QString& currency) const;
    Q_INVOKABLE QString getCurrencySign(const QString& currency) const;
    Q_INVOKABLE bool isSupportedCurrency(const QString& currency) const;
    
    /*Q_INVOKABLE */static void initializeDatabase(); // Cannot be a Q_INVOKABLE since it will only be used in C++
    static std::string getDatabaseHash();
    Q_INVOKABLE QVariantList getCategoryList() const;
    //Q_INVOKABLE QStringList getSubCategoryList(int category_id);
    
    Q_INVOKABLE QVariantList getMoneroNodeList() const;

    QVariantList validateDisplayName(const QString& display_name) const; // Validates display name based on regex requirements
    QVariantList checkDisplayName(const QString& display_name) const; // Checks database for display name availability
    
    Q_INVOKABLE QVariantList/*bool*/ registerUser(const QString& primary_address/*gui::Wallet* wallet*/, const QString& display_name);
    Q_INVOKABLE void loginWithWalletFile();
    Q_INVOKABLE void loginWithMnemonic();
    Q_INVOKABLE void loginWithKeys();
    Q_INVOKABLE void loginWithHW();
    
    //Q_INVOKABLE QVariantList getListings(); // Products listed by sellers
    //Q_INVOKABLE QVariantList getListingsByMostRecent();
    //Q_INVOKABLE QVariantList getListingsByMostFavorited();
    //Q_INVOKABLE QVariantList getListingsByMostSales();
    //Q_INVOKABLE QVariantList getProducts(); // Registered products
    //Q_INVOKABLE QVariantList get();
    
    ////Q_INVOKABLE void listItem();
    //Q_INVOKABLE void addToCart();
    //Q_INVOKABLE void createOrder();
    //Q_INVOKABLE void rateItem();
    //Q_INVOKABLE void rateSeller();
    //Q_INVOKABLE void addToFavorites();
    //Q_INVOKABLE void removeFromFavorites();
    //Q_INVOKABLE void ();
private:
};
}
#endif
