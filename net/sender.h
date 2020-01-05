#ifndef NET_SENDER_H
#define NET_SENDER_H

#include <stdint.h>
#include <time.h>
#include <limits.h>
#include "net/buffers.h"
#include "net/socket.h"
#include "net/ssl/socket.h"

namespace net {
  // Sender.
  class sender {
    public:
      // Constructor.
      sender(buffer::allocator& allocator);

      // Destructor.
      ~sender();

      // Start.
      enum class encryption {
        yes,
        no
      };

      bool start(const char* address, encryption enc, const char* directory);
      bool start(const char* address,
                 in_port_t port,
                 encryption enc,
                 const char* directory);

      // Stop.
      void stop();

      // Send buffer.
      void send(buffer* buf);

      // Send buffers.
      void send(buffer* first, buffer* last);

    private:
      // Idle timeout in seconds (after this time an idle connection is closed).
      static constexpr const time_t idle_timeout = 60;

      // Reconnection time in seconds.
      static constexpr const time_t reconnection_time = 30;

      // Maximum number of queued buffers. Reached this limit, the buffers are
      // written to disk.
      static constexpr const size_t max_queued_buffers = 10000;

      // Socket timeout in milliseconds.
      static constexpr const int socket_timeout = socket::default_timeout;

      // Buffer allocator.
      buffer::allocator& _M_allocator;

      // Socket address.
      socket::address _M_sockaddr;

      // Sockets.
      socket _M_sock;
      ssl::socket _M_sslsock;

      typedef bool (sender::*fnconnect)();
      typedef void (sender::*fndisconnect)();
      typedef bool (sender::*fnsend)(const void*, size_t);
      typedef bool (sender::*fnconnected)() const;
      typedef bool (sender::*fnconnection_closed_by_peer)();

      fnconnect _M_connect;
      fndisconnect _M_disconnect;
      fnsend _M_send;
      fnconnected _M_connected;
      fnconnection_closed_by_peer _M_connection_closed_by_peer;

      // Directory where to store the buffers.
      char _M_directory[PATH_MAX];

      // Buffers.
      buffers _M_buffers;

      // State.
      enum class state {
        sending_files,
        sending_queued_buffers
      };

      state _M_state = state::sending_files;

      // Current time.
      time_t _M_current_time;

      // Time of the last socket operation.
      time_t _M_last_socket_operation = 0;

      // Thread id.
      pthread_t _M_thread;

      // Running?
      bool _M_running = false;

      // Start.
      bool start(encryption enc, const char* directory);

      // Run.
      void run();

      // Connect (insecure).
      bool connect_insecure();

      // Disconnect (insecure).
      void disconnect_insecure();

      // Send (insecure).
      bool send_insecure(const void* buf, size_t len);

      // Connected (insecure)?
      bool connected_insecure() const;

      // Connection closed by peer (insecure)?
      bool connection_closed_by_peer_insecure();

      // Connect (secure).
      bool connect_secure();

      // Disconnect (secure).
      void disconnect_secure();

      // Send (secure).
      bool send_secure(const void* buf, size_t len);

      // Connected (secure)?
      bool connected_secure() const;

      // Connection closed by peer (secure)?
      bool connection_closed_by_peer_secure();

      // Connect.
      bool connect();

      // Disconnect.
      void disconnect();

      // Send.
      bool send(const void* buf, size_t len);

      // Connected?
      bool connected() const;

      // Connection closed by peer?
      bool connection_closed_by_peer();

      // Send files.
      bool send_files();

      // Send file.
      bool send_file(const char* filename, size_t filesize);

      // Save buffers to disk.
      bool save_buffers();

      // Save buffer to disk.
      bool save_buffer(const buffer* buf, unsigned& count);

      // Start routine.
      static void* start_routine(void* arg);

      // Disable copy constructor and assignment operator.
      sender(const sender&) = delete;
      sender& operator=(const sender&) = delete;
  };

  inline sender::sender(buffer::allocator& allocator)
    : _M_allocator(allocator)
  {
  }

  inline sender::~sender()
  {
    stop();
  }

  inline bool net::sender::start(const char* address,
                                 encryption enc,
                                 const char* directory)
  {
    return _M_sockaddr.build(address) ? start(enc, directory) : false;
  }

  inline bool net::sender::start(const char* address,
                                 in_port_t port,
                                 encryption enc,
                                 const char* directory)
  {
    return _M_sockaddr.build(address, port) ? start(enc, directory) : false;
  }

  inline bool sender::connect_insecure()
  {
    return _M_sock.connect(_M_sockaddr, socket_timeout);
  }

  inline void sender::disconnect_insecure()
  {
    _M_sock.close();
  }

  inline bool sender::send_insecure(const void* buf, size_t len)
  {
    return _M_sock.send(buf, len, socket_timeout);
  }

  inline bool sender::connected_insecure() const
  {
    return (_M_sock.fd() != -1);
  }

  inline bool sender::connection_closed_by_peer_insecure()
  {
    uint8_t buf[1024];
    return (_M_sock.recv(buf, sizeof(buf)) == 0);
  }

  inline bool sender::connect_secure()
  {
    return _M_sslsock.connect(_M_sockaddr, socket_timeout);
  }

  inline void sender::disconnect_secure()
  {
    _M_sslsock.close();
  }

  inline bool sender::send_secure(const void* buf, size_t len)
  {
    return _M_sslsock.send(buf, len, socket_timeout);
  }

  inline bool sender::connected_secure() const
  {
    return (_M_sslsock.fd() != -1);
  }

  inline bool sender::connection_closed_by_peer_secure()
  {
    uint8_t buf[1024];
    return (_M_sslsock.recv(buf, sizeof(buf)) == 0);
  }

  inline bool sender::connect()
  {
    // If we are already connected...
    if (connected()) {
      return true;
    } else {
      // Save time of the last socket operation.
      _M_last_socket_operation = _M_current_time;

      // Connect.
      return (this->*_M_connect)();
    }
  }

  inline void sender::disconnect()
  {
    (this->*_M_disconnect)();
  }

  inline bool sender::connected() const
  {
    return (this->*_M_connected)();
  }

  inline bool sender::connection_closed_by_peer()
  {
    return (this->*_M_connection_closed_by_peer)();
  }

  inline void* sender::start_routine(void* arg)
  {
    static_cast<sender*>(arg)->run();
    return nullptr;
  }
}

#endif // NET_SENDER_H
