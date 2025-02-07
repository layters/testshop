#pragma once

#ifndef BACKEND_HPP_NEROSHOP
#define BACKEND_HPP_NEROSHOP

#include <QObject>
#include <QUrl>
#include <QString>
#include <QStringList>

#include "wallet_manager.hpp"
#include "user_manager.hpp"

#include <iostream>

namespace neroshop {
class Backend : public QObject { // This class was created for storing utility functions and backend implementations // Maybe I should rename this to BackendTools?
    Q_OBJECT
public:
    Backend(QObject *parent = nullptr);
    ~Backend();

    //Q_PROPERTY(int categoryProductCount READ getCategoryProductCount NOTIFY categoryProductCountChanged)
    //Q_PROPERTY(QVariantList searchResults READ getListingsBySearchTerm NOTIFY searchResultsChanged)

    Q_INVOKABLE QString urlToLocalFile(const QUrl& url) const;
    Q_INVOKABLE void copyTextToClipboard(const QString& text);
    
    QString imageToBase64(const QImage& image); // un-tested
    QImage base64ToImage(const QString& base64Data); // un-tested
    
    Q_INVOKABLE bool isSupportedImageDimension(int width, int height);
    Q_INVOKABLE bool isSupportedImageSizeBytes(int sizeBytes);
    
    Q_INVOKABLE double weightToKg(double amount, const QString& unit_name) const;
    Q_INVOKABLE double lgToKg(double amount) const;

    Q_INVOKABLE QStringList getCurrencyList() const;
    Q_INVOKABLE int getCurrencyDecimals(const QString& currency) const;
    Q_INVOKABLE QString getCurrencySign(const QString& currency) const;
    Q_INVOKABLE bool isSupportedCurrency(const QString& currency) const;
    
    Q_INVOKABLE QStringList getCountriesList() const;
    Q_INVOKABLE QStringList getRegionsList(const QString& country) const;
    Q_INVOKABLE QStringList getCitiesList(const QString& country, const QString& region) const;
    
    Q_INVOKABLE QString getDurationFromNow(const QString& timestamp) const;
    
    /*Q_INVOKABLE */static void initializeDatabase(); // Cannot be a Q_INVOKABLE since it will only be used in C++
    static std::string getDatabaseHash();
    
    // TODO: Use Q_ENUM for sorting in order by a specific column (e.e Sort.Name, Sort.Id)
    Q_INVOKABLE QVariantList getCategoryList(bool sort_alphabetically = false) const;
    Q_INVOKABLE QVariantList getSubCategoryList(int category_id, bool sort_alphabetically = false) const;
    Q_INVOKABLE int getCategoryIdByName(const QString& category_name) const;
    Q_INVOKABLE int getSubCategoryIdByName(const QString& subcategory_name) const;
    Q_INVOKABLE int getCategoryProductCount(int category_id) const; // returns number of products that fall under a specific category
    Q_INVOKABLE bool hasSubCategory(int category_id) const;
    
    Q_INVOKABLE QVariantList getNodeList(const QString& coin) const;
    Q_INVOKABLE QVariantList getNodeListDefault(const QString& coin) const;
    Q_INVOKABLE bool isWalletDaemonRunning() const;

    QVariantList validateDisplayName(const QString& display_name) const; // Validates display name based on regex requirements
    
    Q_INVOKABLE QVariantList registerUser(WalletManager* wallet_manager, const QString& display_name, UserManager * user_manager, const QVariantMap& avatarMap);
    Q_INVOKABLE int loginWithWalletFile(WalletManager* wallet_manager, const QString& path, const QString& password, UserManager * user_manager);
    Q_INVOKABLE int loginWithMnemonic(WalletManager* wallet_manager, const QString& mnemonic, unsigned int restore_height, UserManager * user_manager);
    Q_INVOKABLE int loginWithKeys(WalletManager* wallet_manager, UserManager * user_manager);
    Q_INVOKABLE int loginWithHW(WalletManager* wallet_manager, UserManager * user_manager);
    
    Q_INVOKABLE QVariantList getListings(int sorting = 0, bool hide_illicit_items = true); // Products listed by sellers
    Q_INVOKABLE QVariantList getListingsByCategory(int category_id, bool hide_illicit_items = true);
    Q_INVOKABLE QVariantList getListingsByMostRecent(int limit, bool hide_illicit_items = true);
    Q_INVOKABLE QVariantList getListingsBySearchTerm(const QString& search_term, bool hide_illicit_items = true); // count is the maximum number of search results (total). The search results (per page) can be between 10-100 or 50-100

    Q_INVOKABLE QVariantList sortBy(const QVariantList& catalog, int sorting = 0);

    Q_INVOKABLE bool saveAvatarImage(const QString& fileName, const QString& userAccountKey);
        
    Q_INVOKABLE QVariantMap uploadImageToObject(const QString& fileName, int imageId); // constructs image object rather than upload it
    Q_INVOKABLE bool saveProductImage(const QString& fileName, const QString& listingKey);
    Q_INVOKABLE bool saveProductThumbnail(const QString& fileName, const QString& listingKey);

    Q_INVOKABLE int getProductStarCount(const QVariantList& product_ratings);
    Q_INVOKABLE int getProductStarCount(const QString& product_id); // getProductRatingsCount
    Q_INVOKABLE int getProductStarCount(const QVariantList& product_ratings, int star_number);
    Q_INVOKABLE int getProductStarCount(const QString& product_id, int star_number);
    Q_INVOKABLE float getProductAverageStars(const QVariantList& product_ratings);
    Q_INVOKABLE float getProductAverageStars(const QString& product_id);
    
    Q_INVOKABLE int getSellerGoodRatings(const QVariantList& seller_ratings);
    Q_INVOKABLE int getSellerGoodRatings(const QString& user_id);
    Q_INVOKABLE int getSellerBadRatings(const QVariantList& seller_ratings);
    Q_INVOKABLE int getSellerBadRatings(const QString& user_id);
    Q_INVOKABLE int getSellerRatingsCount(const QVariantList& seller_ratings);
    Q_INVOKABLE int getSellerRatingsCount(const QString& user_id);
    Q_INVOKABLE int getSellerReputation(const QVariantList& seller_ratings);
    Q_INVOKABLE int getSellerReputation(const QString& user_id);
    // Rating models
    Q_INVOKABLE QVariantList getProductRatings(const QString& product_id/*listing_id*/); // or do I use user account key?
    Q_INVOKABLE QVariantList getSellerRatings(const QString& user_id); // or do I use user account key?
    
    Q_INVOKABLE QString getDisplayNameByUserId(const QString& user_id);
    Q_INVOKABLE QString getKeyByUserId(const QString& user_id);
    // User model
    Q_INVOKABLE QVariantMap getUser(const QString& user_id);
    Q_INVOKABLE int getAccountAge(const QString& userId);
    Q_INVOKABLE int getAccountAge(const QVariantMap& userMap);
    
    Q_INVOKABLE int getCartMaximumItems();
    Q_INVOKABLE int getCartMaximumQuantity();
    
    Q_INVOKABLE int getStockAvailable(const QString& product_id);
    // Inventory model
    Q_INVOKABLE QVariantList getInventory(const QString& user_id, bool hide_illicit_items = true);
    
    Q_INVOKABLE void createOrder(UserManager * user_manager, const QString& shipping_address);

    bool isIllicitItem(const QVariantMap& listing_obj);
    
    Q_INVOKABLE QString getPaymentCoinAsString(int paymentCoin);
    Q_INVOKABLE QString getShippingOptionAsString(int shippingOption);
    
    Q_INVOKABLE QVariantMap getNetworkStatus() const;
    Q_INVOKABLE void clearHashTable();

signals:
    //void categoryProductCountChanged();//(int category_id);
    //void searchResultsChanged();
private:
};
}
#endif
