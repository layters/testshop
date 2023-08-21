#pragma once

#ifndef ENUM_WRAPPER_HPP_NEROSHOP
#define ENUM_WRAPPER_HPP_NEROSHOP

#include <QObject>

namespace neroshop {

class EnumWrapper : public QObject {
    Q_OBJECT
public:
    EnumWrapper(QObject* parent = nullptr);
    
    enum class WalletError {
        Ok = 0, 
        WrongPassword, 
        PasswordsDoNotMatch, 
        AlreadyExists,
        IsOpenedByAnotherProgram,
        DoesNotExist,
        BadNetworkType,
        IsNotOpened, // monero_wallet_obj is nullptr
    };    
    Q_ENUM(WalletError)

    enum class CartError {
        Ok,
        Missing,
        Full,
        ItemOutOfStock,
        ItemQuantitySurpassed,
        ItemQuantityNotSpecified,
        SellerAddOwnItem,
    };
    Q_ENUM(CartError)
    
    enum class LoginError {
        Ok = 0,
    
        WrongPassword = static_cast<int>(WalletError::WrongPassword),
        WalletIsOpenedByAnotherProgram = static_cast<int>(WalletError::IsOpenedByAnotherProgram),
        WalletDoesNotExist = static_cast<int>(WalletError::DoesNotExist),
        WalletBadNetworkType = static_cast<int>(WalletError::BadNetworkType),
        WalletIsNotOpened = static_cast<int>(WalletError::IsNotOpened),
        
        DaemonIsNotConnected = 10,
        
        UserNotFound = 20,
        UserIsNullPointer,
    };
    Q_ENUM(LoginError)

    enum class ListingSorting {
        SortNone = 0,
        SortByCategory,
        SortByMostRecent,//SortByLatest = SortByMostRecent,
        SortByOldest,
        SortByAlphabeticalOrder,
        SortByPriceLowest,
        SortByPriceHighest,
        SortByMostFavorited,
        SortByMostSales,
    };
    Q_ENUM(ListingSorting)
    
    enum class InventorySorting {
        SortNone = 0,
        SortByAvailability, // If item is in stock        
        SortByDateOldest,
        SortByDateNewest,
        SortByName,
        SortByQuantitySmallest,
        SortByQuantityBiggest,
        SortByPriceLowest,
        SortByPriceHighest,
        SortByProductCode,
        SortByCategory,
        SortByCondition,
        //TODO: productid, currency, location, color, size, weight, imagefilesize, desc
    };
    Q_ENUM(InventorySorting)
};
    
}

Q_DECLARE_METATYPE(neroshop::EnumWrapper::WalletError)
Q_DECLARE_METATYPE(neroshop::EnumWrapper::CartError)
Q_DECLARE_METATYPE(neroshop::EnumWrapper::LoginError)
Q_DECLARE_METATYPE(neroshop::EnumWrapper::ListingSorting)
Q_DECLARE_METATYPE(neroshop::EnumWrapper::InventorySorting)
#endif
