#pragma once

#ifndef ESCROW_HPP_NEROSHOP
#define ESCROW_HPP_NEROSHOP

#include <iostream>
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>

#include "../wallet/wallet.hpp"

namespace neroshop {

struct Escrow {
    static void test_multisig_participants(const std::vector<neroshop::Wallet *>& participants, int M, int N, bool test_tx);
    static void test_create_multisig_wallet(int M, int N);
    // Multi-sig functions to use:
    //std::string monero::monero_wallet_full::export_multisig_hex 	( 		) 	
    //monero_multisig_init_result monero::monero_wallet_full::exchange_multisig_keys 	( 	const std::vector< std::string > &  	mutisig_hexes,const std::string &  	password )
    //std::vector< std::string > monero::monero_wallet_full::submit_multisig_tx_hex 	( 	const std::string &  	signed_multisig_tx_hex	) 	
    //monero_multisig_info monero::monero_wallet_full::get_multisig_info 	( 		) 	const
    //int monero::monero_wallet_full::import_multisig_hex 	( 	const std::vector< std::string > &  	multisig_hexes	) 	
    //bool monero::monero_wallet_full::is_multisig_import_needed 	( 		) 	const
    //std::string monero::monero_wallet_full::make_multisig 	( 	const std::vector< std::string > &  	multisig_hexes,int  	threshold,const std::string &  	password ) 	
    //std::string monero::monero_wallet_full::prepare_multisig 	( 		) 	
    //monero_multisig_sign_result monero::monero_wallet_full::sign_multisig_tx_hex 	( 	const std::string &  	multisig_tx_hex	) 	
    //monero_tx_set monero::monero_wallet_full::describe_tx_set 	( 	const monero_tx_set &  	tx_set	) 	
};

}

#endif
