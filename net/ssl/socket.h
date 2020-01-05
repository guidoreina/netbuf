#ifndef NET_SSL_SOCKET_H
#define NET_SSL_SOCKET_H

#include <openssl/ssl.h>
#include "net/socket.h"

namespace net {
  namespace ssl {
    class socket {
      public:
        // Set context object.
        static void context(SSL_CTX* ctx);

        // Constructor.
        socket() = default;
        socket(net::socket& sock);

        // Destructor.
        ~socket();

        // Close socket.
        void close();

        // Shutdown socket.
        enum class shutdown_how {
          unidirectional,
          bidirectional
        };

        bool shutdown(shutdown_how how, int timeout);

        // Connect and perform handshake.
        bool connect(const net::socket::address& addr, int timeout);
        bool connect(const net::socket::address::ipv4& addr, int timeout);
        bool connect(const net::socket::address::ipv6& addr, int timeout);
        bool connect(const net::socket::address::local& addr, int timeout);

        // Perform handshake.
        enum class mode {
          client,
          server
        };

        bool handshake(mode m, int timeout);

        // Receive.
        ssize_t recv(void* buf, size_t len);
        ssize_t recv(void* buf, size_t len, int timeout);

        // Send.
        bool send(const void* buf, size_t len);
        bool send(const void* buf, size_t len, int timeout);

        // Get socket descriptor.
        int fd() const;

      private:
        // Context object.
        static SSL_CTX* _M_ctx;

        net::socket _M_sock;

        SSL* _M_ssl = nullptr;

        // Connect.
        bool connect(int timeout);

        // Perform handshake.
        bool handshake(int timeout);

        // Disable copy constructor and assignment operator.
        socket(const socket&) = delete;
        socket& operator=(const socket&) = delete;
    };

    inline void socket::context(SSL_CTX* ctx)
    {
      _M_ctx = ctx;
    }

    inline socket::socket(net::socket& sock)
      : _M_sock(sock)
    {
    }

    inline socket::~socket()
    {
      close();
    }

    inline void socket::close()
    {
      _M_sock.close();

      if (_M_ssl) {
        SSL_free(_M_ssl);
        _M_ssl = nullptr;
      }
    }

    inline bool socket::connect(const net::socket::address& addr, int timeout)
    {
      return ((_M_sock.connect(addr, timeout)) && (connect(timeout)));
    }

    inline bool socket::connect(const net::socket::address::ipv4& addr,
                                int timeout)
    {
      return ((_M_sock.connect(addr, timeout)) && (connect(timeout)));
    }

    inline bool socket::connect(const net::socket::address::ipv6& addr,
                                int timeout)
    {
      return ((_M_sock.connect(addr, timeout)) && (connect(timeout)));
    }

    inline bool socket::connect(const net::socket::address::local& addr,
                                int timeout)
    {
      return ((_M_sock.connect(addr, timeout)) && (connect(timeout)));
    }

    inline int socket::fd() const
    {
      return _M_sock.fd();
    }
  }
}

#endif // NET_SSL_SOCKET_H
