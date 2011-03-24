/* Copyright (c) 2010 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_DHT_TRANSPORT_UDT_MULTIPLEXER_H_
#define MAIDSAFE_DHT_TRANSPORT_UDT_MULTIPLEXER_H_

#ifdef __MSVC__
#pragma warning(disable:4996)
#endif
#include <memory>
#ifdef __MSVC__
#pragma warning(default:4996)
#endif

#include <unordered_map>
#include <vector>
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/ip/udp.hpp"
#include "maidsafe-dht/transport/transport.h"
#include "maidsafe-dht/transport/udt_accept_op.h"

namespace maidsafe {

namespace transport {

class UdtAcceptor;
class UdtSocket;

class UdtMultiplexer : public std::enable_shared_from_this<UdtMultiplexer> {
 public:
  UdtMultiplexer(boost::asio::io_service &asio_service);
  ~UdtMultiplexer();

  // Open the multiplexer as a server on the specified endpoint.
  TransportCondition Open(const Endpoint &endpoint);

  // Stop listening for incoming connections and terminate all connections.
  void Close();

  // Create a new acceptor. Only one is allowed at a time.
  std::shared_ptr<UdtAcceptor> NewAcceptor();

  // Create a new client-side connection.
  std::shared_ptr<UdtSocket> NewClient(const Endpoint &endpoint);

 private:
  friend class UdtAcceptor;
  friend class UdtSocket;

  // Disallow copying and assignment.
  UdtMultiplexer(const UdtMultiplexer&);
  UdtMultiplexer &operator=(const UdtMultiplexer&);

  void StartReceive();
  void HandleReceive(const boost::system::error_code &ec,
                     size_t bytes_transferred);

  // Called by the acceptor or socket objects to send a packet. Returns true if
  // the data was sent successfully, false otherwise.
  bool SendTo(const boost::asio::const_buffer &data,
              const boost::asio::ip::udp::endpoint &endpoint);

  // The UDP socket used for all UDT protocol communication.
  boost::asio::ip::udp::socket socket_;

  // Data members used to receive information about incoming packets.
  static const size_t kMaxPacketSize = 1500;
  std::vector<unsigned char> receive_buffer_;
  boost::asio::ip::udp::endpoint sender_endpoint_;

  // The one-and-only acceptor.
  std::weak_ptr<UdtAcceptor> udt_acceptor_;

  // Map of destination socket id to corresponding socket object.
  typedef std::unordered_map<boost::uint32_t,
                             std::weak_ptr<UdtSocket>> SocketMap;
  SocketMap udt_sockets_;
};

}  // namespace transport

}  // namespace maidsafe

#endif  // MAIDSAFE_DHT_TRANSPORT_UDT_MULTIPLEXER_H_