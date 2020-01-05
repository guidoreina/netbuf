#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

namespace net {
  class socket {
    public:
      class address {
        friend class socket;

        public:
          class ipv4 {
            public:
              // Constructor.
              ipv4();
              ipv4(const struct sockaddr_in& addr);

              // Get size.
              static socklen_t size();

              // Get family.
              static sa_family_t family();

              // Get port.
              in_port_t port() const;

              // Set port.
              void port(in_port_t p);

              // Build.
              bool build(const char* address, in_port_t port);
              bool build(const char* address);

              // Comparison operator.
              bool operator==(const ipv4& addr) const;
              bool operator==(const struct sockaddr_in& addr) const;

              // Assignment operator.
              ipv4& operator=(const ipv4& addr);
              ipv4& operator=(const struct sockaddr_in& addr);

              // To string.
              const char* to_string(char* s, size_t n) const;

              // Cast operators.
              operator const struct sockaddr*() const;
              operator struct sockaddr*();

              operator const struct sockaddr&() const;
              operator struct sockaddr&();

            private:
              struct sockaddr_in _M_addr;
          };

          class ipv6 {
            public:
              // Constructor.
              ipv6();
              ipv6(const struct sockaddr_in6& addr);

              // Get size.
              static socklen_t size();

              // Get family.
              static sa_family_t family();

              // Get port.
              in_port_t port() const;

              // Set port.
              void port(in_port_t p);

              // Build.
              bool build(const char* address, in_port_t port);
              bool build(const char* address);

              // Comparison operator.
              bool operator==(const ipv6& addr) const;
              bool operator==(const struct sockaddr_in6& addr) const;

              // Assignment operator.
              ipv6& operator=(const ipv6& addr);
              ipv6& operator=(const struct sockaddr_in6& addr);

              // To string.
              const char* to_string(char* s, size_t n) const;

              // Cast operators.
              operator const struct sockaddr*() const;
              operator struct sockaddr*();

              operator const struct sockaddr&() const;
              operator struct sockaddr&();

            private:
              struct sockaddr_in6 _M_addr;
          };

          class local {
            public:
              // Constructor.
              local();
              local(const struct sockaddr_un& addr);

              // Get size.
              static socklen_t size();

              // Get family.
              static sa_family_t family();

              // Build.
              bool build(const char* address);
              bool build(const void* address, size_t len);

              // Comparison operator.
              bool operator==(const local& addr) const;
              bool operator==(const struct sockaddr_un& addr) const;

              // Assignment operator.
              local& operator=(const local& addr);
              local& operator=(const struct sockaddr_un& addr);

              // To string.
              const char* to_string(char* s, size_t n) const;

              // Cast operators.
              operator const struct sockaddr*() const;
              operator struct sockaddr*();

              operator const struct sockaddr&() const;
              operator struct sockaddr&();

            private:
              struct sockaddr_un _M_addr;
          };

          // Constructor.
          address() = default;

          // Get size.
          socklen_t size() const;

          // Get family.
          sa_family_t family() const;

          // Build.
          bool build(const char* address, in_port_t port);
          bool build(const char* address);

          // Comparison operator.
          bool operator==(const address& addr) const;
          bool operator==(const struct sockaddr& addr) const;

          // Equal?
          static bool equal(const struct sockaddr_in& addr1,
                            const struct sockaddr_in& addr2);

          // Equal?
          static bool equal(const struct sockaddr_in6& addr1,
                            const struct sockaddr_in6& addr2);

          // Equal?
          static bool equal(const struct in6_addr& addr1,
                            const struct in6_addr& addr2);

          // Equal?
          static bool equal(const struct sockaddr_un& addr1,
                            const struct sockaddr_un& addr2);

          // Assignment operator.
          address& operator=(const address& addr);
          address& operator=(const struct sockaddr& addr);

          // To string.
          const char* to_string(char* s, size_t n) const;
          static const char* to_string(const struct sockaddr_in& addr,
                                       char* s,
                                       size_t n);

          static const char* to_string(const struct sockaddr_in6& addr,
                                       char* s,
                                       size_t n);

          static const char* to_string(const struct sockaddr_un& addr,
                                       char* s,
                                       size_t n);

          // Cast operators.
          operator const struct sockaddr*() const;
          operator struct sockaddr*();

          operator const struct sockaddr&() const;
          operator struct sockaddr&();

        private:
          struct sockaddr_storage _M_addr;
          socklen_t _M_addrlen = sizeof(struct sockaddr_storage);
      };

      static constexpr const int default_timeout = 30 * 1000; // Milliseconds.

      // Constructor.
      socket() = default;
      socket(int fd);

      // Constructor (takes ownership of the socket).
      socket(socket& sock);

      // Destructor.
      ~socket();

      // Close socket.
      void close();

      // Shutdown socket.
      enum class shutdown_how {
        read       = SHUT_RD,
        write      = SHUT_WR,
        read_write = SHUT_RDWR
      };

      bool shutdown(shutdown_how how);

      // Connect.
      bool connect(const address& addr);
      bool connect(const address::ipv4& addr);
      bool connect(const address::ipv6& addr);
      bool connect(const address::local& addr);
      bool connect(const address& addr, int timeout);
      bool connect(const address::ipv4& addr, int timeout);
      bool connect(const address::ipv6& addr, int timeout);
      bool connect(const address::local& addr, int timeout);

      // Get socket error.
      bool get_socket_error(int& error);

      // Get receive buffer size.
      bool get_recvbuf_size(int& size);

      // Set receive buffer size.
      bool set_recvbuf_size(int size);

      // Get send buffer size.
      bool get_sendbuf_size(int& size);

      // Set send buffer size.
      bool set_sendbuf_size(int size);

      // Get keep-alive.
      bool get_keep_alive(bool& on);

      // Set keep-alive.
      bool set_keep_alive(bool on);

      // Get TCP no delay.
      bool get_tcp_no_delay(bool& on);

      // Set TCP no delay.
      bool set_tcp_no_delay(bool on);

      // Cork.
      bool cork();

      // Uncork.
      bool uncork();

      // Listen.
      bool listen(const address& addr);
      bool listen(const address::ipv4& addr);
      bool listen(const address::ipv6& addr);
      bool listen(const address::local& addr);

      // Accept.
      bool accept(socket& sock, address& addr);
      bool accept(socket& sock);
      bool accept(socket& sock, address& addr, int timeout);
      bool accept(socket& sock, int timeout);

      // Receive.
      ssize_t recv(void* buf, size_t len);
      ssize_t recv(void* buf, size_t len, int timeout);

      // Send.
      ssize_t send(const void* buf, size_t len);
      bool send(const void* buf, size_t len, int timeout);

      // Wait for the socket to be readable.
      bool wait_readable(int timeout);

      // Wait for the socket to be writable.
      bool wait_writable(int timeout);

      // Get socket descriptor.
      int fd() const;

      // Set socket descriptor.
      void fd(int n);

    private:
      int _M_fd = -1;

      // Connect.
      bool connect(const struct sockaddr& addr, socklen_t addrlen);
      bool connect(const struct sockaddr& addr, socklen_t addrlen, int timeout);

      // Listen.
      bool listen(const sockaddr& addr, socklen_t addrlen);

      // Accept.
      bool accept(socket& sock, struct sockaddr* addr, socklen_t* addrlen);
      bool accept(socket& sock,
                  struct sockaddr* addr,
                  socklen_t* addrlen,
                  int timeout);

      // Disable copy constructor and assignment operator.
      socket(const socket&) = delete;
      socket& operator=(const socket&) = delete;
  };


  //////////////////////////////////////
  //////////////////////////////////////
  //                                  //
  // Class ipv4.                      //
  //                                  //
  //////////////////////////////////////
  //////////////////////////////////////

  inline socket::address::ipv4::ipv4()
  {
    _M_addr.sin_family = AF_INET;
    memset(_M_addr.sin_zero, 0, sizeof(_M_addr.sin_zero));
  }

  inline socket::address::ipv4::ipv4(const struct sockaddr_in& addr)
  {
    _M_addr = addr;
  }

  inline socklen_t socket::address::ipv4::size()
  {
    return sizeof(struct sockaddr_in);
  }

  inline sa_family_t socket::address::ipv4::family()
  {
    return AF_INET;
  }

  inline in_port_t socket::address::ipv4::port() const
  {
    return ntohs(_M_addr.sin_port);
  }

  inline void socket::address::ipv4::port(in_port_t p)
  {
    _M_addr.sin_port = htons(p);
  }

  inline bool socket::address::ipv4::operator==(const ipv4& addr) const
  {
    return equal(_M_addr, addr._M_addr);
  }

  inline
  bool socket::address::ipv4::operator==(const struct sockaddr_in& addr) const
  {
    return equal(_M_addr, addr);
  }

  inline
  socket::address::ipv4& socket::address::ipv4::operator=(const ipv4& addr)
  {
    _M_addr.sin_addr.s_addr = addr._M_addr.sin_addr.s_addr;
    _M_addr.sin_port = addr._M_addr.sin_port;

    return *this;
  }

  inline socket::address::ipv4&
  socket::address::ipv4::operator=(const struct sockaddr_in& addr)
  {
    _M_addr.sin_addr.s_addr = addr.sin_addr.s_addr;
    _M_addr.sin_port = addr.sin_port;

    return *this;
  }

  inline const char* socket::address::ipv4::to_string(char* s, size_t n) const
  {
    return address::to_string(_M_addr, s, n);
  }

  inline socket::address::ipv4::operator const struct sockaddr*() const
  {
    return reinterpret_cast<const struct sockaddr*>(&_M_addr);
  }

  inline socket::address::ipv4::operator struct sockaddr*()
  {
    return reinterpret_cast<struct sockaddr*>(&_M_addr);
  }

  inline socket::address::ipv4::operator const struct sockaddr&() const
  {
    return reinterpret_cast<const struct sockaddr&>(_M_addr);
  }

  inline socket::address::ipv4::operator struct sockaddr&()
  {
    return reinterpret_cast<struct sockaddr&>(_M_addr);
  }


  //////////////////////////////////////
  //////////////////////////////////////
  //                                  //
  // Class ipv6.                      //
  //                                  //
  //////////////////////////////////////
  //////////////////////////////////////

  inline socket::address::ipv6::ipv6()
  {
    _M_addr.sin6_family = AF_INET6;

    _M_addr.sin6_flowinfo = 0;
    _M_addr.sin6_scope_id = 0;
  }

  inline socket::address::ipv6::ipv6(const struct sockaddr_in6& addr)
  {
    _M_addr = addr;
  }

  inline socklen_t socket::address::ipv6::size()
  {
    return sizeof(struct sockaddr_in6);
  }

  inline sa_family_t socket::address::ipv6::family()
  {
    return AF_INET6;
  }

  inline in_port_t socket::address::ipv6::port() const
  {
    return ntohs(_M_addr.sin6_port);
  }

  inline void socket::address::ipv6::port(in_port_t p)
  {
    _M_addr.sin6_port = htons(p);
  }

  inline bool socket::address::ipv6::operator==(const ipv6& addr) const
  {
    return equal(_M_addr, addr._M_addr);
  }

  inline
  bool socket::address::ipv6::operator==(const struct sockaddr_in6& addr) const
  {
    return equal(_M_addr, addr);
  }

  inline
  socket::address::ipv6& socket::address::ipv6::operator=(const ipv6& addr)
  {
    _M_addr.sin6_port = addr._M_addr.sin6_port;
    _M_addr.sin6_flowinfo = addr._M_addr.sin6_flowinfo;
    _M_addr.sin6_addr = addr._M_addr.sin6_addr;
    _M_addr.sin6_scope_id = addr._M_addr.sin6_scope_id;

    return *this;
  }

  inline socket::address::ipv6&
  socket::address::ipv6::operator=(const struct sockaddr_in6& addr)
  {
    _M_addr.sin6_port = addr.sin6_port;
    _M_addr.sin6_flowinfo = addr.sin6_flowinfo;
    _M_addr.sin6_addr = addr.sin6_addr;
    _M_addr.sin6_scope_id = addr.sin6_scope_id;

    return *this;
  }

  inline const char* socket::address::ipv6::to_string(char* s, size_t n) const
  {
    return address::to_string(_M_addr, s, n);
  }

  inline socket::address::ipv6::operator const struct sockaddr*() const
  {
    return reinterpret_cast<const struct sockaddr*>(&_M_addr);
  }

  inline socket::address::ipv6::operator struct sockaddr*()
  {
    return reinterpret_cast<struct sockaddr*>(&_M_addr);
  }

  inline socket::address::ipv6::operator const struct sockaddr&() const
  {
    return reinterpret_cast<const struct sockaddr&>(_M_addr);
  }

  inline socket::address::ipv6::operator struct sockaddr&()
  {
    return reinterpret_cast<struct sockaddr&>(_M_addr);
  }


  //////////////////////////////////////
  //////////////////////////////////////
  //                                  //
  // Class local.                     //
  //                                  //
  //////////////////////////////////////
  //////////////////////////////////////

  inline socket::address::local::local()
  {
    _M_addr.sun_family = AF_UNIX;
  }

  inline socket::address::local::local(const struct sockaddr_un& addr)
  {
    _M_addr = addr;
  }

  inline socklen_t socket::address::local::size()
  {
    return sizeof(struct sockaddr_un);
  }

  inline sa_family_t socket::address::local::family()
  {
    return AF_UNIX;
  }

  inline bool socket::address::local::operator==(const local& addr) const
  {
    return equal(_M_addr, addr._M_addr);
  }

  inline
  bool socket::address::local::operator==(const struct sockaddr_un& addr) const
  {
    return equal(_M_addr, addr);
  }

  inline
  socket::address::local& socket::address::local::operator=(const local& addr)
  {
    memcpy(_M_addr.sun_path, addr._M_addr.sun_path, sizeof(_M_addr.sun_path));

    return *this;
  }

  inline socket::address::local&
  socket::address::local::operator=(const struct sockaddr_un& addr)
  {
    memcpy(_M_addr.sun_path, addr.sun_path, sizeof(_M_addr.sun_path));

    return *this;
  }

  inline const char* socket::address::local::to_string(char* s, size_t n) const
  {
    return address::to_string(_M_addr, s, n);
  }

  inline socket::address::local::operator const struct sockaddr*() const
  {
    return reinterpret_cast<const struct sockaddr*>(&_M_addr);
  }

  inline socket::address::local::operator struct sockaddr*()
  {
    return reinterpret_cast<struct sockaddr*>(&_M_addr);
  }

  inline socket::address::local::operator const struct sockaddr&() const
  {
    return reinterpret_cast<const struct sockaddr&>(_M_addr);
  }

  inline socket::address::local::operator struct sockaddr&()
  {
    return reinterpret_cast<struct sockaddr&>(_M_addr);
  }


  //////////////////////////////////////
  //////////////////////////////////////
  //                                  //
  // Class address.                   //
  //                                  //
  //////////////////////////////////////
  //////////////////////////////////////

  inline socklen_t socket::address::size() const
  {
    return _M_addrlen;
  }

  inline sa_family_t socket::address::family() const
  {
    return _M_addr.ss_family;
  }

  inline bool socket::address::operator==(const address& addr) const
  {
    return ((_M_addrlen == addr._M_addrlen) &&
            (memcmp(&_M_addr, &addr._M_addr, _M_addrlen) == 0));
  }

  inline bool socket::address::equal(const struct sockaddr_in& addr1,
                                     const struct sockaddr_in& addr2)
  {
    return ((addr1.sin_port == addr2.sin_port) &&
            (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr));
  }

  inline bool socket::address::equal(const struct sockaddr_in6& addr1,
                                     const struct sockaddr_in6& addr2)
  {
    return ((addr1.sin6_port == addr2.sin6_port) &&
            (equal(addr1.sin6_addr, addr2.sin6_addr)));
  }

  inline bool socket::address::equal(const struct in6_addr& addr1,
                                     const struct in6_addr& addr2)
  {
    return (((addr1.s6_addr32[0] ^ addr2.s6_addr32[0]) |
             (addr1.s6_addr32[1] ^ addr2.s6_addr32[1]) |
             (addr1.s6_addr32[2] ^ addr2.s6_addr32[2]) |
             (addr1.s6_addr32[3] ^ addr2.s6_addr32[3])) == 0);
  }

  inline bool socket::address::equal(const struct sockaddr_un& addr1,
                                     const struct sockaddr_un& addr2)
  {
    return addr1.sun_path[0] ? (strcmp(addr1.sun_path,
                                       addr2.sun_path) == 0) :
                               (memcmp(addr1.sun_path,
                                       addr2.sun_path,
                                       sizeof(addr1.sun_path)) == 0);
  }

  inline socket::address& socket::address::operator=(const address& addr)
  {
    memcpy(&_M_addr, &addr._M_addr, addr._M_addrlen);
    _M_addrlen = addr._M_addrlen;

    return *this;
  }

  inline socket::address::operator const struct sockaddr*() const
  {
    return reinterpret_cast<const struct sockaddr*>(&_M_addr);
  }

  inline socket::address::operator struct sockaddr*()
  {
    return reinterpret_cast<struct sockaddr*>(&_M_addr);
  }

  inline socket::address::operator const struct sockaddr&() const
  {
    return reinterpret_cast<const struct sockaddr&>(_M_addr);
  }

  inline socket::address::operator struct sockaddr&()
  {
    return reinterpret_cast<struct sockaddr&>(_M_addr);
  }


  //////////////////////////////////////
  //////////////////////////////////////
  //                                  //
  // Class socket.                    //
  //                                  //
  //////////////////////////////////////
  //////////////////////////////////////

  inline socket::socket(int fd)
    : _M_fd(fd)
  {
  }

  inline socket::socket(socket& sock)
    : _M_fd(sock._M_fd)
  {
    sock._M_fd = -1;
  }

  inline socket::~socket()
  {
    close();
  }

  inline void socket::close()
  {
    if (_M_fd != -1) {
      ::close(_M_fd);
      _M_fd = -1;
    }
  }

  inline bool socket::shutdown(shutdown_how how)
  {
    return (::shutdown(_M_fd, static_cast<int>(how)) == 0);
  }

  inline bool socket::connect(const address& addr)
  {
    return connect(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::connect(const address::ipv4& addr)
  {
    return connect(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::connect(const address::ipv6& addr)
  {
    return connect(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::connect(const address::local& addr)
  {
    return connect(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::connect(const address& addr, int timeout)
  {
    return connect(static_cast<const struct sockaddr&>(addr),
                   addr.size(),
                   timeout);
  }

  inline bool socket::connect(const address::ipv4& addr, int timeout)
  {
    return connect(static_cast<const struct sockaddr&>(addr),
                   addr.size(),
                   timeout);
  }

  inline bool socket::connect(const address::ipv6& addr, int timeout)
  {
    return connect(static_cast<const struct sockaddr&>(addr),
                   addr.size(),
                   timeout);
  }

  inline bool socket::connect(const address::local& addr, int timeout)
  {
    return connect(static_cast<const struct sockaddr&>(addr),
                   addr.size(),
                   timeout);
  }

  inline bool socket::get_socket_error(int& error)
  {
    socklen_t optlen = sizeof(int);
    return (getsockopt(_M_fd, SOL_SOCKET, SO_ERROR, &error, &optlen) == 0);
  }

  inline bool socket::get_recvbuf_size(int& size)
  {
    socklen_t optlen = sizeof(int);
    return (getsockopt(_M_fd, SOL_SOCKET, SO_RCVBUF, &size, &optlen) == 0);
  }

  inline bool socket::set_recvbuf_size(int size)
  {
    return (setsockopt(_M_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) == 0);
  }

  inline bool socket::get_sendbuf_size(int& size)
  {
    socklen_t optlen = sizeof(int);
    return (getsockopt(_M_fd, SOL_SOCKET, SO_SNDBUF, &size, &optlen) == 0);
  }

  inline bool socket::set_sendbuf_size(int size)
  {
    return (setsockopt(_M_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int)) == 0);
  }

  inline bool socket::get_keep_alive(bool& on)
  {
    int optval;
    socklen_t optlen = sizeof(int);

    if (getsockopt(_M_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) == 0) {
      on = (optval != 0);
      return true;
    }

    return false;
  }

  inline bool socket::set_keep_alive(bool on)
  {
    const int optval = on;
    return (setsockopt(_M_fd,
                       SOL_SOCKET,
                       SO_KEEPALIVE,
                       &optval,
                       sizeof(int)) == 0);
  }

  inline bool socket::get_tcp_no_delay(bool& on)
  {
    int optval;
    socklen_t optlen = sizeof(int);

    if (getsockopt(_M_fd, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen) == 0) {
      on = (optval != 0);
      return true;
    }

    return false;
  }

  inline bool socket::set_tcp_no_delay(bool on)
  {
    const int optval = on;
    return (setsockopt(_M_fd,
                       IPPROTO_TCP,
                       TCP_NODELAY,
                       &optval,
                       sizeof(int)) == 0);
  }

  inline bool socket::cork()
  {
    static constexpr const int optval = 1;
    return (setsockopt(_M_fd,
                       IPPROTO_TCP,
                       TCP_CORK,
                       &optval,
                       sizeof(int)) == 0);
  }

  inline bool socket::uncork()
  {
    static constexpr const int optval = 0;
    return (setsockopt(_M_fd,
                       IPPROTO_TCP,
                       TCP_CORK,
                       &optval,
                       sizeof(int)) == 0);
  }

  inline bool socket::listen(const address& addr)
  {
    return listen(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::listen(const address::ipv4& addr)
  {
    return listen(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::listen(const address::ipv6& addr)
  {
    return listen(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::listen(const address::local& addr)
  {
    return listen(static_cast<const struct sockaddr&>(addr), addr.size());
  }

  inline bool socket::accept(socket& sock, address& addr)
  {
    socklen_t addrlen = addr.size();
    if (accept(sock, static_cast<struct sockaddr*>(addr), &addrlen)) {
      addr._M_addrlen = addrlen;
      return true;
    }

    return false;
  }

  inline bool socket::accept(socket& sock)
  {
    return accept(sock, nullptr, nullptr);
  }

  inline bool socket::accept(socket& sock, address& addr, int timeout)
  {
    socklen_t addrlen = addr.size();
    if (accept(sock, static_cast<struct sockaddr*>(addr), &addrlen, timeout)) {
      addr._M_addrlen = addrlen;
      return true;
    }

    return false;
  }

  inline bool socket::accept(socket& sock, int timeout)
  {
    return accept(sock, nullptr, nullptr, timeout);
  }

  inline int socket::fd() const
  {
    return _M_fd;
  }

  inline void socket::fd(int n)
  {
    _M_fd = n;
  }
}

#endif // NET_SOCKET_H
