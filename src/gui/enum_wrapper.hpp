#pragma once

#ifndef ENUM_WRAPPER_HPP_NEROSHOP
#define ENUM_WRAPPER_HPP_NEROSHOP

#include <QObject>

namespace neroshop {

class EnumWrapper : public QObject {
    Q_OBJECT
public:
    EnumWrapper(QObject* parent = nullptr);
    
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

    enum class ListingSorting {
        SortNone = 0,
        SortByCategory,
        SortByMostRecent,
        SortByOldest,
        SortByAlphabeticalOrder,
        SortByPriceLowest,
        SortByPriceHighest,
        SortByMostFavorited,
        SortByMostSales,
    };
    Q_ENUM(ListingSorting)
};
    
}

Q_DECLARE_METATYPE(neroshop::EnumWrapper::CartError)
Q_DECLARE_METATYPE(neroshop::EnumWrapper::ListingSorting)
#endif
