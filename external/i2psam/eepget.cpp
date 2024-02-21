/**
 * Copyright (c) 2017 The I2P Project
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or http://www.opensource.org/licenses/mit-license.php.
 *
 * See full documentation about SAM at http://www.i2p2.i2p/samv3.html
 */

#include "i2psam.h"

int main(int argc, char **argv)
{
  if (argc < 2)
    {
      std::cerr << "Usage: eepget <hostname.i2p>" << std::endl;
      return 1;
    }

  std::string target(argv[1]);
  SAM::StreamSession s("eepget");
  auto lookupResult = s.namingLookup(target);
  auto connResult = s.connect(lookupResult.value, false);
  auto conn = connResult.value.get();
  conn->write("GET / HTTP/1.1\r\n\r\n");
  auto reply = conn->read();

  while (!reply.empty())
    {
      std::cout << reply << std::flush;
      reply = conn->read();
    }

  conn->close();
}
