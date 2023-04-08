#include "query_builder.hpp"
#include "listing.hpp"
#include "debug.hpp"

std::string neroshop::QueryBuilder::create_limit_clause(const neroshop::GenericSearchParams& params) {
    return create_limit_clause(params.page, params.count);
}

std::string neroshop::QueryBuilder::create_limit_clause(int page, int count) {
    if(page < 1) {
        page = 1;
    }
    if(count < 1 && count > MAX_SEARCH_COUNT) {
        count = DEFAULT_SEARCH_COUNT;
    }
    return std::string("LIMIT ")+std::to_string((page-1)*count)+std::string(", ")+std::to_string(count);
}

std::string neroshop::QueryBuilder::create_order_by_clause(const neroshop::GenericSearchParams& params) {
    const std::string direction = params.order_by_ascending ? "ASC" : "DESC";
    const std::string collate = params.collate_order_by ? "COLLATE " : "";
    const std::string no_case = params.no_case_order_by ? "NOCASE " : "";
    //The order by column cannot be passed a prepared statement variable,
    //so we need to check against every valid column name before allowing column_name to be used.
    //Note: Some columns require the table name to be prepended because they have matching names
    //across tables and would otherwise cause an error for being ambiguous.
    if(params.order_by_column.length() > 0) {
        const char* column_name = params.order_by_column.c_str();
        if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "." STRINGIFY(PRODUCT_ID_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "." STRINGIFY(PRODUCT_NAME_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_DESCRIPTION_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_ATTRIBUTES_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_CODE_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_CATEGORY_ID_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(SELLER_TABLE_NAME) "." STRINGIFY(SELLER_ID_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(SELLER_TABLE_NAME) "." STRINGIFY(SELLER_NAME_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(SELLER_KEY_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(SELLER_AVATAR_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_TABLE_NAME) "." STRINGIFY(LISTING_ID_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_PRODUCT_ID_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_SELLER_ID_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_QUANTITY_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_PRICE_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_CURRENCY_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_CONDITION_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_LOCATION_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(LISTING_DATE_COLUMN_NAME), column_name)) {
            return std::string("ORDER BY ")+column_name+std::string(" ")+collate+no_case+direction;
        } else if(!strcmp(STRINGIFY(PRODUCT_RATING_COLUMN_NAME), column_name)
        || !strcmp(STRINGIFY(PRODUCT_RATING_COUNT_COLUMN_NAME), column_name)) {
            return std::string("ORDER BY ")+std::string(STRINGIFY(PRODUCT_TABLE_NAME) "_")+column_name+std::string(" ")+collate+no_case+direction;
        }
        neroshop::print(std::string("Invalid column was provided to QueryBuilder: '")+column_name+std::string("'."), 2);
    }
    return std::string("");
}

std::string neroshop::QueryBuilder::create_seller_search_where_clause(const neroshop::GenericSearchParams& params) {
    std::string clause("");
    std::vector<std::string> terms = neroshop::string::split(neroshop::string::trim(params.terms), " ");
    if(terms.size() > 0) {
        clause += "( ";
    }
    for(std::string term : terms) {
        if(clause.length() > 2) {
            clause += std::string(" OR ");
        }
        clause += std::string(STRINGIFY(SELLER_NAME_COLUMN_NAME))+std::string(" LIKE ?");
        clause += std::string(" OR ")+std::string(STRINGIFY(SELLER_ID_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(terms.size() > 0) {
        clause += std::string(" )");
    }
    if(params.id.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(SELLER_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.name.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(SELLER_NAME_COLUMN_NAME))+std::string(" = ?");
    }
    if(clause.length() > 0) {
        clause = std::string("WHERE ")+clause;
    } else {
        clause = std::string("WHERE TRUE");
    }
    return clause;
}

std::string neroshop::QueryBuilder::create_product_search_where_clause(const ProductSearchParams& params) {
    std::string clause("");
    std::vector<std::string> terms = neroshop::string::split(neroshop::string::trim(params.terms), " ");
    if(terms.size() > 0) {
        clause += "( ";
    }
    for(std::string term : terms) {
        if(clause.length() > 2) {
            clause += std::string(" OR ");
        }
        clause += std::string(STRINGIFY(PRODUCT_NAME_COLUMN_NAME))+std::string(" LIKE ?");
        clause += std::string(" OR ")+std::string(STRINGIFY(PRODUCT_DESCRIPTION_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(terms.size() > 0) {
        clause += std::string(" )");
    }
    if(params.id.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.name.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_NAME_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.min_weight > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME))+std::string(" >= ?");
    }
    if(params.max_weight > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME))+std::string(" <= ?");
    }
    if(params.attributes.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_ATTRIBUTES_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(params.code.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_CODE_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.category_id > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_CATEGORY_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.location.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_LOCATION_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(params.date.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_DATE_COLUMN_NAME))+std::string(" = ?");
    }
    if(clause.length() > 0) {
        clause = std::string("WHERE ")+clause;
    } else {
        clause = std::string("WHERE TRUE");
    }
    return clause;
}

std::string neroshop::QueryBuilder::create_listing_search_where_clause(const ListingSearchParams& params) {
    std::string clause("");
    std::vector<std::string> terms = neroshop::string::split(neroshop::string::trim(params.terms), " ");
    if(terms.size() > 0) {
        clause += "( ";
    }
    for(std::string term : terms) {
        if(clause.length() > 2) {
            clause += std::string(" OR ");
        }
        clause += std::string(STRINGIFY(PRODUCT_TABLE_NAME) "." STRINGIFY(PRODUCT_NAME_COLUMN_NAME))+std::string(" LIKE ?");
        clause += std::string(" OR ")+std::string(STRINGIFY(PRODUCT_DESCRIPTION_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(terms.size() > 0) {
        clause += std::string(" )");
    }
    if(params.id.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_TABLE_NAME) "." STRINGIFY(LISTING_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.name.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_TABLE_NAME) "." STRINGIFY(PRODUCT_NAME_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.min_weight > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME))+std::string(" >= ?");
    }
    if(params.max_weight > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME))+std::string(" <= ?");
    }
    if(params.attributes.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_ATTRIBUTES_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(params.code.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_CODE_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.category_id > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_CATEGORY_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.location.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_LOCATION_COLUMN_NAME))+std::string(" LIKE ?");
    }
    if(params.date.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(PRODUCT_DATE_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.product_id.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_PRODUCT_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.seller_id.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_SELLER_ID_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.quantity > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_QUANTITY_COLUMN_NAME))+std::string(" >= ?");
    }
    if(params.min_price > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_PRICE_COLUMN_NAME))+std::string(" >= ?");
    }
    if(params.max_price > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_PRICE_COLUMN_NAME))+std::string(" <= ?");
    }
    if(params.currency.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_CURRENCY_COLUMN_NAME))+std::string(" = ?");
    }
    if(params.condition.length() > 0) {
        if(clause.length() > 0) {
            clause += std::string(" AND ");
        }
        clause += std::string(STRINGIFY(LISTING_CONDITION_COLUMN_NAME))+std::string(" = ?");
    }
    if(clause.length() > 0) {
        clause = std::string("WHERE ")+clause;
    } else {
        clause = std::string("WHERE TRUE");
    }
    return clause;
}

std::string neroshop::QueryBuilder::create_product_column_clause(const bool use_count) {
    const std::string rating_column_name = std::string(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_RATING_COLUMN_NAME));
    const std::string rating_count_column_name = std::string(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_RATING_COUNT_COLUMN_NAME));
    if(use_count) {
        return std::string("COUNT(*), 0 AS ")
            +rating_column_name
            +std::string(", 0 AS ")
            +rating_count_column_name;
    } else {
        const std::string rating_sub_query = std::string(", (SELECT AVG(")
                +std::string(STRINGIFY(PRODUCT_RATINGS_STARS_COLUMN_NAME))
            +std::string(") FROM ")
                +std::string(STRINGIFY(PRODUCT_RATINGS_TABLE_NAME))
            +std::string(" WHERE ")
                +std::string(STRINGIFY(PRODUCT_RATINGS_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_RATINGS_PRODUCT_ID_COLUMN_NAME))
                +std::string(" = ")
                +std::string(STRINGIFY(PRODUCT_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_ID_COLUMN_NAME))
            +std::string(" GROUP BY ")
                +std::string(STRINGIFY(PRODUCT_RATINGS_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_RATINGS_PRODUCT_ID_COLUMN_NAME))
            +std::string(") AS ")
                +rating_column_name;
        const std::string rating_count_sub_query = std::string(", (SELECT COUNT(*) ")
            +std::string(" FROM ")
                +std::string(STRINGIFY(PRODUCT_RATINGS_TABLE_NAME))
            +std::string(" WHERE ")
                +std::string(STRINGIFY(PRODUCT_RATINGS_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_RATINGS_PRODUCT_ID_COLUMN_NAME))
                +std::string(" = ")
                +std::string(STRINGIFY(PRODUCT_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_ID_COLUMN_NAME))
            +std::string(" GROUP BY ")
                +std::string(STRINGIFY(PRODUCT_RATINGS_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_RATINGS_PRODUCT_ID_COLUMN_NAME))
            +std::string(") AS ")
                +rating_count_column_name;
        return neroshop::Product::get_table_names_for_query()+rating_sub_query+rating_count_sub_query;
    }
}

std::string neroshop::QueryBuilder::create_listing_column_clause(const bool use_count) {
    if(use_count) {
        return neroshop::QueryBuilder::create_product_column_clause(use_count);
    } else {
        return neroshop::QueryBuilder::create_product_column_clause(use_count)+std::string(", ")
            +neroshop::Listing::get_table_names_for_query()+std::string(", ")
            +neroshop::ListingSeller::get_table_names_for_query();
    }
}

std::string neroshop::QueryBuilder::create_seller_column_clause(const bool use_count) {
    if(use_count) {
        return std::string("COUNT(*)");
    } else {
        return std::string(neroshop::ListingSeller::get_table_names_for_query());
    }
}

std::string neroshop::QueryBuilder::create_total_results_product_search_query(const neroshop::ProductSearchParams& params) {
    return neroshop::QueryBuilder::create_product_search_query(params, true);
}

std::string neroshop::QueryBuilder::create_product_search_query(const neroshop::ProductSearchParams& params, const bool use_count) {
    const std::string limit_clause = use_count ? "" : neroshop::QueryBuilder::create_limit_clause(params);
    return std::string("SELECT ")
            +neroshop::QueryBuilder::create_product_column_clause(use_count)
        +std::string(" FROM ")
            +std::string(STRINGIFY(PRODUCT_TABLE_NAME))
        +std::string(" ")
            +neroshop::QueryBuilder::create_product_search_where_clause(params)
        +std::string(" ")
            +neroshop::QueryBuilder::create_order_by_clause(params)
        +std::string(" ")
            +limit_clause;
}

std::string neroshop::QueryBuilder::create_total_results_listing_search_query(const neroshop::ListingSearchParams& params) {
    return neroshop::QueryBuilder::create_listing_search_query(params, true);
}

std::string neroshop::QueryBuilder::create_listing_search_query(const neroshop::ListingSearchParams& params, const bool use_count) {
    const std::string limit_clause = use_count ? "" : neroshop::QueryBuilder::create_limit_clause(params);
    return std::string("SELECT ")
            +neroshop::QueryBuilder::create_listing_column_clause(use_count)
        +std::string(" FROM ")
            +std::string(STRINGIFY(LISTING_TABLE_NAME))
        +std::string(" JOIN ")
            +std::string(STRINGIFY(PRODUCT_TABLE_NAME))
        +std::string(" ON ")
            +std::string(STRINGIFY(LISTING_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(LISTING_PRODUCT_ID_COLUMN_NAME))
            +std::string(" = ")
            +std::string(STRINGIFY(PRODUCT_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(PRODUCT_ID_COLUMN_NAME))
        +std::string(" JOIN ")
            +std::string(STRINGIFY(SELLER_TABLE_NAME))
        +std::string(" ON ")
            +std::string(STRINGIFY(SELLER_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(SELLER_ID_COLUMN_NAME))
            +std::string(" = ")
            +std::string(STRINGIFY(LISTING_TABLE_NAME))+std::string(".")+std::string(STRINGIFY(LISTING_SELLER_ID_COLUMN_NAME))
        +std::string(" ")
            +neroshop::QueryBuilder::create_listing_search_where_clause(params)
        +std::string(" ")
            +neroshop::QueryBuilder::create_order_by_clause(params)
        +std::string(" ")
            +limit_clause;
}

std::string neroshop::QueryBuilder::create_total_results_seller_search_query(const neroshop::GenericSearchParams& params) {
    return neroshop::QueryBuilder::create_seller_search_query(params, true);
}

std::string neroshop::QueryBuilder::create_seller_search_query(const neroshop::GenericSearchParams& params, const bool use_count) {
    const std::string limit_clause = use_count ? "" : neroshop::QueryBuilder::create_limit_clause(params);
    return std::string("SELECT ")
            +neroshop::QueryBuilder::create_seller_column_clause(use_count)
        +std::string(" FROM ")
            +std::string(STRINGIFY(SELLER_TABLE_NAME))
        +std::string(" ")
            +neroshop::QueryBuilder::create_seller_search_where_clause(params)
        +std::string(" ")
            +neroshop::QueryBuilder::create_order_by_clause(params)
        +std::string(" ")
            +limit_clause;
}
