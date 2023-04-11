#include <iostream>
#include <cassert> // assert
//#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <uuid.h>
//#include <catch.hpp>
//#endif

int main() {
    // Creating a new UUID (using system generator)
    #ifdef UUID_SYSTEM_GENERATOR
    std::cout << "using UUID_SYSTEM_GENERATOR\n";
    uuids::uuid const id = uuids::uuid_system_generator{}();
    assert(!id.is_nil());
    assert(id.version() == uuids::uuid_version::random_number_based);
    assert(id.variant() == uuids::uuid_variant::rfc);
    
    std::cout << uuids::to_string(id) << std::endl;
    #else
    // Creating a new UUID with a default random generator
    //std::cout << "using default random generator\n";
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
    
    std::cout << uuids::to_string(id) << std::endl;
    #endif 
    return 0;   
} // todo: make uuids human-readable
