#pragma once

#ifndef I2P_HPP_NEROSHOP
#define I2P_HPP_NEROSHOP

#include <api.h>
#include <Base.h>
#include <Blinding.h>
#include <ChaCha20.h>
#include <Config.h>
#include <CPU.h>
#include <Crypto.h>
#include <CryptoKey.h>
#include <Datagram.h>
#include <Destination.h>
#include <ECIESX25519AEADRatchetSession.h>
#include <Ed25519.h>
#include <Elligator.h>
#include <Family.h>
#include <FS.h>
#include <Garlic.h>
#include <Gost.h>
#include <Gzip.h>
#include <HTTP.h>
#include <I2NPProtocol.h>
#include <I2PEndian.h>
#include <Identity.h>
#include <KadDHT.h>
#include <LeaseSet.h>
//#include <LittleBigEndian.h>
#include <Log.h>
#include <NetDb.hpp>
#include <NetDbRequests.h>
#include <NTCP2.h>
#include <Poly1305.h>
#include <Profiling.h>
//#include <Queue.h>
#include <Reseed.h>
#include <RouterContext.h>
#include <RouterInfo.h>
#include <Signature.h>
//#include <Siphash.h>
#include <SSU2.h>
#include <SSU2Session.h>
#include <Streaming.h>
//#include <Tag.h>
#include <Timestamp.h>
#include <TransitTunnel.h>
#include <Transports.h>
//#include <TransportSession.h>
#include <Tunnel.h>
//#include <TunnelBase.h>
#include <TunnelConfig.h>
#include <TunnelEndpoint.h>
#include <TunnelGateway.h>
#include <TunnelPool.h>
#include <util.h>
//#include <version.h>

#include <SAM.h>

namespace neroshop {

    bool is_i2p_running();

}
#endif
