#ifndef QUERY_BUILDER_HPP_NEROSHOP
#define QUERY_BUILDER_HPP_NEROSHOP

#include "search_params.hpp"

namespace neroshop {
class QueryBuilder {
private:
    QueryBuilder() {}

protected:
    static std::string create_limit_clause(const GenericSearchParams& params);

    static std::string create_limit_clause(int page, int count);

    static std::string create_order_by_clause(const GenericSearchParams& params);

    static std::string create_seller_search_where_clause(const GenericSearchParams& params);

    static std::string create_product_search_where_clause(const ProductSearchParams& params);

    static std::string create_listing_search_where_clause(const ListingSearchParams& params);

    static std::string create_product_column_clause(const bool use_count);

    static std::string create_listing_column_clause(const bool use_count);

    static std::string create_seller_column_clause(const bool use_count);

public:
    static std::string create_total_results_product_search_query(const ProductSearchParams& params);

    static std::string create_product_search_query(const ProductSearchParams& params, const bool use_count = false);

    static std::string create_total_results_listing_search_query(const ListingSearchParams& params);

    static std::string create_listing_search_query(const ListingSearchParams& params, const bool use_count = false);

    static std::string create_total_results_seller_search_query(const GenericSearchParams& params);

    static std::string create_seller_search_query(const GenericSearchParams& params, const bool use_count = false);
};
}
#endif
