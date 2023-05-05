#pragma once

#if defined(NEROSHOP_USE_QT)
#include <QUuid>
#else
#include <uuid.h>
//#include <catch.hpp>
#endif

#include <cassert>

namespace neroshop {

namespace uuid {
    static std::string generate() {
        std::string uuid_out = "";
        #if defined(NEROSHOP_USE_QT)
        QString quuid = QUuid::createUuid().toString();
        quuid = quuid.remove("{").remove("}"); // remove brackets
        
        uuid_out = quuid.toStdString();
        #else
        // Creating a new UUID with a default random generator
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        uuids::uuid_random_generator gen{generator};

        uuids::uuid const id = gen();
        assert(!id.is_nil());
        assert(id.as_bytes().size() == 16);
        assert(id.version() == uuids::uuid_version::random_number_based);
        assert(id.variant() == uuids::uuid_variant::rfc);
    
        uuid_out = uuids::to_string(id);
        #endif
        return uuid_out;
    }
} // g++ uuid.hpp -I../../../external/stduuid/include -I../../../external/stduuid/

}
