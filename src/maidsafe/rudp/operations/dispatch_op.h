/*******************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of MaidSafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of MaidSafe.net. *
 ******************************************************************************/
// Original author: Christopher M. Kohlhoff (chris at kohlhoff dot com)

#ifndef MAIDSAFE_RUDP_OPERATIONS_DISPATCH_OP_H_
#define MAIDSAFE_RUDP_OPERATIONS_DISPATCH_OP_H_

#include <mutex>
#include "boost/asio/buffer.hpp"
#include "boost/asio/handler_alloc_hook.hpp"
#include "boost/asio/handler_invoke_hook.hpp"
#include "boost/system/error_code.hpp"
#include "maidsafe/rudp/core/dispatcher.h"

namespace maidsafe {

namespace rudp {

namespace detail {

// Helper class to perform an asynchronous dispatch operation.
template <typename DispatchHandler>
class DispatchOp {
 public:
  DispatchOp(DispatchHandler handler,
             boost::asio::ip::udp::socket& socket,
             const boost::asio::mutable_buffer& buffer,
             boost::asio::ip::udp::endpoint& sender_endpoint,
             Dispatcher& dispatcher)
      : handler_(handler),
        socket_(socket),
        buffer_(buffer),
        mutex_(std::make_shared<std::mutex>()),
        sender_endpoint_(sender_endpoint),
        dispatcher_(dispatcher) {}

  DispatchOp(const DispatchOp& other)
      : handler_(other.handler_),
        socket_(other.socket_),
        buffer_(other.buffer_),
        mutex_(other.mutex_),
        sender_endpoint_(other.sender_endpoint_),
        dispatcher_(other.dispatcher_) {}

  void operator()(const boost::system::error_code& ec, size_t bytes_transferred) {
    boost::system::error_code local_ec = ec;
    while (!local_ec) {
      std::lock_guard<std::mutex> lock(*mutex_);
      dispatcher_.HandleReceiveFrom(boost::asio::buffer(buffer_, bytes_transferred),
                                    sender_endpoint_);
      bytes_transferred = socket_.receive_from(boost::asio::buffer(buffer_),
                                               sender_endpoint_,
                                               0,
                                               local_ec);
    }

    handler_(ec);
  }

  friend void* asio_handler_allocate(size_t n, DispatchOp* op) {
    using boost::asio::asio_handler_allocate;
    return asio_handler_allocate(n, &op->handler_);
  }

  friend void asio_handler_deallocate(void* p, size_t n, DispatchOp* op) {
    using boost::asio::asio_handler_deallocate;
    asio_handler_deallocate(p, n, &op->handler_);
  }

  template <typename Function>
  friend void asio_handler_invoke(const Function& f, DispatchOp* op) {
    using boost::asio::asio_handler_invoke;
    asio_handler_invoke(f, &op->handler_);
  }

 private:
  // Disallow assignment.
  DispatchOp& operator=(const DispatchOp&);

  DispatchHandler handler_;
  boost::asio::ip::udp::socket& socket_;
  boost::asio::mutable_buffer buffer_;
  std::shared_ptr<std::mutex> mutex_;
  boost::asio::ip::udp::endpoint& sender_endpoint_;
  Dispatcher& dispatcher_;
};

}  // namespace detail

}  // namespace rudp

}  // namespace maidsafe

#endif  // MAIDSAFE_RUDP_OPERATIONS_DISPATCH_OP_H_
