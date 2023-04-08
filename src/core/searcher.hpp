#ifndef SEARCHER_HPP_NEROSHOP
#define SEARCHER_HPP_NEROSHOP

#include "search_params.hpp"
#include "listing.hpp"

#define ITERATION_LIMIT 500

namespace neroshop {
class Searcher {
private:
    Searcher(){};

public:
    static int seller_search_total_results(const GenericSearchParams& params);

    static std::vector<ListingSeller> seller_search(const GenericSearchParams& params);

    static std::vector<ListingSeller> find_seller_by_id(int seller_id);

    static int seller_search_by_name_total_results(std::string name);

    static std::vector<ListingSeller> seller_search_by_name(std::string name, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int product_search_total_results(const ProductSearchParams& params);

    static std::vector<Product> product_search(const ProductSearchParams& params);

    static std::vector<Product> product_search_by_id(int product_id);

    static int product_search_by_code_total_results(std::string code);

    static std::vector<Product> product_search_by_code(std::string code, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int product_search_by_category_id_total_results(int category_id);

    static std::vector<Product> product_search_by_category_id(int category_id, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int product_search_by_highest_rating_total_results();

    static std::vector<Product> product_search_by_highest_rating(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int product_search_by_most_rated_total_results();

    static std::vector<Product> product_search_by_most_rated(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_total_results(const ListingSearchParams& params);

    static std::vector<Listing> listing_search(const ListingSearchParams& params);

    static int listing_search_by_product_id_total_results(std::string product_id);

    static std::vector<Listing> listing_search_by_product_id(std::string product_id, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_seller_id_total_results(std::string seller_id);

    static std::vector<Listing> listing_search_by_seller_id(std::string seller_id, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_seller_total_results(ListingSeller seller);

    static std::vector<Listing> listing_search_by_seller(ListingSeller seller, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_category_id_total_results(int category_id);

    static std::vector<Listing> listing_search_by_category_id(int category_id, int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_most_recent_total_results();

    static std::vector<Listing> listing_search_by_most_recent(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_oldest_total_results();

    static std::vector<Listing> listing_search_by_oldest(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_alphabetical_order_total_results();

    static std::vector<Listing> listing_search_by_alphabetical_order(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_price_lowest_total_results();

    static std::vector<Listing> listing_search_by_price_lowest(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);

    static int listing_search_by_price_highest_total_results();

    static std::vector<Listing> listing_search_by_price_highest(int page = DEFAULT_SEARCH_PAGE, int count = DEFAULT_SEARCH_COUNT);
};
}
#endif
