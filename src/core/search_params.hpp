#ifndef SEARCH_PARAMS_HPP_NEROSHOP
#define SEARCH_PARAMS_HPP_NEROSHOP

#include <string>
#include <vector>
#include "sqlite3.h"
#include "util.hpp"

#if defined(NEROSHOP_BUILD_GUI)
#include <QObject>
#endif

#define DEFAULT_SEARCH_PAGE 1

#define DEFAULT_SEARCH_COUNT 15
#define MAX_SEARCH_COUNT 300

namespace neroshop {
struct GenericSearchParams {
    int page;
    int count;
    std::string order_by_column;
    bool order_by_ascending;
    bool collate_order_by;
    bool no_case_order_by;
    std::string terms;
    std::string id;
    std::string name;

    GenericSearchParams() : page(DEFAULT_SEARCH_PAGE), count(DEFAULT_SEARCH_COUNT), order_by_column(""), order_by_ascending(true), collate_order_by(false), no_case_order_by(false), terms(""), id(""), name("") {}

    int bind_params(sqlite3_stmt* statement) const;
};

struct ProductSearchParams : public GenericSearchParams {
    double min_weight;
    double max_weight;
    std::string attributes;
    std::string code;
    int category_id;
    std::string location;
    std::string date;

    ProductSearchParams() : GenericSearchParams(), min_weight(-1), max_weight(-1), attributes(""), code(""), category_id(-1), location(""), date("") {}

    int bind_params(sqlite3_stmt* statement) const;
};

struct ListingSearchParams : public ProductSearchParams {
    std::string product_id;
    std::string seller_id;
    int quantity;
    double min_price;
    double max_price;
    std::string currency;
    std::string condition;

    ListingSearchParams() : ProductSearchParams(), product_id(""), seller_id(""), quantity(-1), min_price(-1), max_price(-1), currency(""), condition("") {}

    int bind_params(sqlite3_stmt* statement) const;
};

#if defined(NEROSHOP_BUILD_GUI)
class QListingSearchParamsWrapper : public QObject {
    Q_OBJECT
private:
    ListingSearchParams params;

public:
    QListingSearchParamsWrapper(QObject *parent = nullptr){}
    Q_INVOKABLE void reset() {
        params = ListingSearchParams();
    }
    ListingSearchParams getParams() {
        return params;
    }
    Q_PROPERTY(int page READ get_page WRITE set_page NOTIFY page_changed)
    Q_INVOKABLE int get_page() {
        return params.page;
    }
    Q_INVOKABLE void set_page(int in) {
        params.page = in;
    }
    Q_PROPERTY(int count READ get_count WRITE set_count NOTIFY count_changed)
    Q_INVOKABLE int get_count() {
        return params.count;
    }
    Q_INVOKABLE void set_count(int in) {
        params.count = in;
    }
    Q_PROPERTY(QString order_by_column READ get_order_by_column WRITE set_order_by_column)
    Q_INVOKABLE QString get_order_by_column() {
        return QString::fromStdString(params.order_by_column);
    }
    Q_INVOKABLE void set_order_by_column(QString in) {
        params.order_by_column = in.toStdString();
    }
    Q_PROPERTY(bool order_by_ascending READ get_order_by_ascending WRITE set_order_by_ascending)
    Q_INVOKABLE bool get_order_by_ascending() {
        return params.order_by_ascending;
    }
    Q_INVOKABLE void set_order_by_ascending(bool in) {
        params.order_by_ascending = in;
    }
    Q_PROPERTY(QString terms READ get_terms WRITE set_terms)
    Q_INVOKABLE QString get_terms() {
        return QString::fromStdString(params.terms);
    }
    Q_INVOKABLE void set_terms(QString in) {
        params.terms = in.toStdString();
    }
    Q_PROPERTY(int category_id READ get_category_id WRITE set_category_id)
    Q_INVOKABLE int get_category_id() {
        return params.category_id;
    }
    Q_INVOKABLE void set_category_id(int in) {
        params.category_id = in;
    }
signals:
    void page_changed(int);
    void count_changed(int);
};
#endif
}
#endif
