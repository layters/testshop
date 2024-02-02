#include <unistd.h>
#include "../src/core/wallet.hpp"

int main() {
    
    neroshop::Wallet *wallet = new neroshop::Wallet();
    wallet->create_random("", "", "random_wallet");
    
    // This isn't working??
    wallet->daemon_connect_remote("node2.monerodevs.org", "38089");
    
    // Your vector of payment addresses
    std::vector<std::pair<std::string, double>> payment_addresses = {
        {"Address1", 1.0},
        {"Address2", 0.5},
        {"Address3", 0.25}
    };
    
    sleep(900); // wait for node to finish syncing?
    
    wallet->transfer(payment_addresses);
    
    return 0;
}
