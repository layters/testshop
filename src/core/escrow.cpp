#include "escrow.hpp"

void neroshop::Escrow::test_multisig_participants(const std::vector<neroshop::Wallet *>& participants, int M, int N, bool test_tx) {
    std::cout << "test_multisig_participants(" + std::to_string(M) + ", " + std::to_string(N) + ")\n";
    assert((N == participants.size()) && std::string("Number of participants must be equal to N (" + std::to_string(N) + ")").c_str());
}

void neroshop::Escrow::test_create_multisig_wallet(int M, int N) {
    std::cout << "Creating " + std::to_string(M) + "/" + std::to_string(N) + " multisig wallet\n";
    
    // create participating wallets
    std::vector<neroshop::Wallet *> wallets = {};
    for(int i = 0; i < N; i++) {
        neroshop::Wallet *wallet = new neroshop::Wallet();
        wallet->create_random("", "", std::string("multisig_wallet_" + std::to_string(i)));
        wallets.push_back(wallet);
    }
    
    // prepare and collect multisig hex from each participant
    std::vector<std::string> prepared_multisig_hexes = {};
    for(auto wallet : wallets) {
        prepared_multisig_hexes.push_back(wallet->get_monero_wallet()->prepare_multisig());
    }
    
    // make each wallet multisig and collect results
    std::vector<std::string> made_multisig_hexes = {};
    for(int i = 0; i < wallets.size(); i++) {
    
        // collect prepared multisig hexes from wallet's peers
        std::vector<std::string> peer_multisig_hexes = {};
        for(int j = 0; j < wallets.size(); j++)
            if(j != i) peer_multisig_hexes.push_back(prepared_multisig_hexes[j]);
        
        // make wallet multisig and collect result hex
      std::string multisig_hex = wallets[i]->get_monero_wallet()->make_multisig(peer_multisig_hexes, M, ""/*password*/);
      made_multisig_hexes.push_back(multisig_hex);    
    }
    
    // exchange multisig keys N - M + 1 times - error is found here!
    std::vector<std::string> multisig_hexes = made_multisig_hexes;
    for (int i = 0; i < N - M + 1; i++) {
        
        // exchange multisig keys among participants and collect results for next round if applicable
        std::vector<std::string> result_multisig_hexes = {};
        for(auto wallet : wallets) {
        
            // import the multisig hex of other participants and collect results
            monero_multisig_init_result result = wallet->get_monero_wallet()->exchange_multisig_keys(multisig_hexes, ""/*password*/);
            result_multisig_hexes.push_back(result.m_multisig_hex.get());
        }
        
        // use resulting multisig hex for next round of exchange if applicable
        multisig_hexes = result_multisig_hexes;
    }
    
    // wallets are now multisig
    for(auto wallet : wallets) {
        std::string primary_address = wallet->get_monero_wallet()->get_address(0, 0);  
        if(!monero_utils::is_valid_address(primary_address, wallet->get_network_type())) {
            throw std::runtime_error("wallet address is not valid!");
        }
        std::cout << "multisig address: " << primary_address << std::endl;
        monero_multisig_info info = wallet->get_monero_wallet()->get_multisig_info();
        assert(info.m_is_multisig == true);
        assert(info.m_is_ready == true);
        assert(M == static_cast<int>(info.m_threshold));
        assert(N == static_cast<int>(info.m_num_participants));
        wallet->get_monero_wallet()->close(true);
    }
}

/*int main() {
    //neroshop::Escrow::test_create_multisig_wallet(2, 3); // can be disputed if both buyer and seller do not come to an agreement (for a fee of 0.5% of order total)
    neroshop::Escrow::test_create_multisig_wallet(2, 2); // funds are gone forever if both buyer and seller do not come to an agreement

    return 0;
}*/
