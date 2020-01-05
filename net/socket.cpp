#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include "net/socket.h"

namespace net {
  static bool parse_port(const char* s, in_port_t& port)
  {
    unsigned n = 0;
    while (*s) {
      if ((*s >= '0') &&
          (*s <= '9') &&
          ((n = (n * 10) + (*s - '0')) <= 65535)) {
        s++;
      } else {
        return false;
      }
    }

    if (n > 0) {
      port = static_cast<in_port_t>(n);
      return true;
    }

    return false;
  }

  bool extract_ip_port(const char* address, char* ip, in_port_t& port)
  {
    // Search last colon.
    const char* colon = nullptr;
    const char* p = address;
    while (*p) {
      if (*p == ':') {
        colon = p;
      }

      p++;
    }

    if (colon) {
      size_t len = colon - address;
      if (len > 0) {
        if (*address == '[') {
          if ((len > 2) && (colon[-1] == ']')) {
            // Skip '['.
            address++;

            len -= 2;
          } else {
            return false;
          }
        }

        if (len < INET6_ADDRSTRLEN) {
          if (parse_port(colon + 1, port)) {
            memcpy(ip, address, len);
            ip[len] = 0;

            return true;
          }
        }
      }
    }

    return false;
  }
}

bool net::socket::address::ipv4::build(const char* address, in_port_t port)
{
  if (inet_pton(AF_INET, address, &_M_addr.sin_addr) == 1) {
    _M_addr.sin_port = htons(port);
    return true;
  }

  return false;
}

bool net::socket::address::ipv4::build(const char* address)
{
  char ip[INET6_ADDRSTRLEN];
  in_port_t port;
  return ((extract_ip_port(address, ip, port)) && (build(ip, port)));
}

bool net::socket::address::ipv6::build(const char* address, in_port_t port)
{
  if (inet_pton(AF_INET6, address, &_M_addr.sin6_addr) == 1) {
    _M_addr.sin6_port = htons(port);
    return true;
  }

  return false;
}

bool net::socket::address::ipv6::build(const char* address)
{
  char ip[INET6_ADDRSTRLEN];
  in_port_t port;
  return ((extract_ip_port(address, ip, port)) && (build(ip, port)));
}

bool net::socket::address::local::build(const char* address)
{
  const size_t len = strlen(address);
  if ((len > 0) && (len < sizeof(_M_addr.sun_path))) {
    memcpy(_M_addr.sun_path, address, len);
    memset(_M_addr.sun_path + len, 0, sizeof(_M_addr.sun_path) - len);

    return true;
  }

  return false;
}

bool net::socket::address::local::build(const void* address, size_t len)
{
  if ((len > 0) && (len <= sizeof(_M_addr.sun_path))) {
    memcpy(_M_addr.sun_path, address, len);
    memset(_M_addr.sun_path + len, 0, sizeof(_M_addr.sun_path) - len);

    return true;
  }

  return false;
}

bool net::socket::address::build(const char* address, in_port_t port)
{
  // IPv4?
  struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&_M_addr);
  if (inet_pton(AF_INET, address, &addr->sin_addr) == 1) {
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    memset(addr->sin_zero, 0, sizeof(addr->sin_zero));

    _M_addrlen = sizeof(struct sockaddr_in);

    return true;
  } else {
    // IPv6?
    struct sockaddr_in6*
      addr = reinterpret_cast<struct sockaddr_in6*>(&_M_addr);

    if (inet_pton(AF_INET6, address, &addr->sin6_addr) == 1) {
      addr->sin6_family = AF_INET6;

      addr->sin6_port = htons(port);
      addr->sin6_flowinfo = 0;
      addr->sin6_scope_id = 0;

      _M_addrlen = sizeof(struct sockaddr_in6);

      return true;
    }
  }

  return false;
}

bool net::socket::address::build(const char* address)
{
  char ip[INET6_ADDRSTRLEN];
  in_port_t port;
  if (extract_ip_port(address, ip, port)) {
    return build(ip, port);
  } else {
    struct sockaddr_un* addr = reinterpret_cast<struct sockaddr_un*>(&_M_addr);
    const size_t len = strlen(address);
    if ((len > 0) && (len < sizeof(addr->sun_path))) {
      addr->sun_family = AF_UNIX;

      memcpy(addr->sun_path, address, len);
      memset(addr->sun_path + len, 0, sizeof(addr->sun_path) - len);

      _M_addrlen = sizeof(struct sockaddr_un);

      return true;
    }
  }

  return false;
}

bool net::socket::address::operator==(const struct sockaddr& addr) const
{
  if (_M_addr.ss_family == addr.sa_family) {
    switch (_M_addr.ss_family) {
      case AF_INET:
        return equal(*reinterpret_cast<const struct sockaddr_in*>(&_M_addr),
                     *reinterpret_cast<const struct sockaddr_in*>(&addr));
      case AF_INET6:
        return equal(*reinterpret_cast<const struct sockaddr_in6*>(&_M_addr),
                     *reinterpret_cast<const struct sockaddr_in6*>(&addr));
      case AF_UNIX:
        return equal(*reinterpret_cast<const struct sockaddr_un*>(&_M_addr),
                     *reinterpret_cast<const struct sockaddr_un*>(&addr));
    }
  }

  return false;
}

net::socket::address&
net::socket::address::operator=(const struct sockaddr& addr)
{
  switch (addr.sa_family) {
    case AF_INET:
      memcpy(&_M_addr, &addr, sizeof(struct sockaddr_in));
      _M_addrlen = sizeof(struct sockaddr_in);

      break;
    case AF_INET6:
      memcpy(&_M_addr, &addr, sizeof(struct sockaddr_in6));
      _M_addrlen = sizeof(struct sockaddr_in6);

      break;
    case AF_UNIX:
      memcpy(&_M_addr, &addr, sizeof(struct sockaddr_un));
      _M_addrlen = sizeof(struct sockaddr_un);

      break;
  }

  return *this;
}

const char* net::socket::address::to_string(char* s, size_t n) const
{
  switch (_M_addr.ss_family) {
    case AF_INET:
      return to_string(*reinterpret_cast<const struct sockaddr_in*>(&_M_addr),
                       s,
                       n);
    case AF_INET6:
      return to_string(*reinterpret_cast<const struct sockaddr_in6*>(&_M_addr),
                       s,
                       n);
    case AF_UNIX:
      if (_M_addrlen > sizeof(sa_family_t)) {
        return to_string(*reinterpret_cast<const struct sockaddr_un*>(&_M_addr),
                         s,
                         n);
      }
  }

  return nullptr;
}

const char* net::socket::address::to_string(const struct sockaddr_in& addr,
                                            char* s,
                                            size_t n)
{
  if (inet_ntop(AF_INET, &addr.sin_addr, s, n)) {
    const size_t len = strlen(s);
    const size_t left = n - len;

    if (snprintf(s + len,
                 left,
                 ":%u",
                 ntohs(addr.sin_port)) < static_cast<ssize_t>(left)) {
      return s;
    } else {
      errno = ENOSPC;
    }
  }

  return nullptr;
}

const char* net::socket::address::to_string(const struct sockaddr_in6& addr,
                                            char* s,
                                            size_t n)
{
  if (n > 1) {
    if (inet_ntop(AF_INET6, &addr.sin6_addr, s + 1, n - 1)) {
      const size_t len = 1 + strlen(s + 1);
      const size_t left = n - len;

      if (snprintf(s + len,
                   left,
                   "]:%u",
                   ntohs(addr.sin6_port)) < static_cast<ssize_t>(left)) {
        *s = '[';

        return s;
      } else {
        errno = ENOSPC;
      }
    }
  } else {
    errno = ENOSPC;
  }

  return nullptr;
}

const char* net::socket::address::to_string(const struct sockaddr_un& addr,
                                            char* s,
                                            size_t n)
{
  const size_t len = strlen(addr.sun_path);
  if (len > 0) {
    if (len < n) {
      memcpy(s, addr.sun_path, len);
      s[len] = 0;

      return s;
    } else {
      errno = ENOSPC;
    }
  } else {
    if (sizeof(addr.sun_path) <= n) {
      memcpy(s, addr.sun_path, sizeof(addr.sun_path));

      return s;
    } else {
      errno = ENOSPC;
    }
  }

  return nullptr;
}

ssize_t net::socket::recv(void* buf, size_t len)
{
  do {
    // Receive.
    const ssize_t ret = ::recv(_M_fd, buf, len, 0);

    // If data has been received or the peer has closed the connection...
    if (ret >= 0) {
      return ret;
    } else {
      if (errno != EINTR) {
        return -1;
      }
    }
  } while (true);
}

ssize_t net::socket::recv(void* buf, size_t len, int timeout)
{
  do {
    // Receive.
    const ssize_t ret = recv(buf, len);

    // If data has been received or the peer has closed the connection...
    if (ret >= 0) {
      return ret;
    } else {
      // If the receive operation would block...
      if (errno == EAGAIN) {
        if (wait_readable(timeout)) {
          return recv(buf, len);
        } else {
          return -1;
        }
      } else {
        return -1;
      }
    }
  } while (true);
}

ssize_t net::socket::send(const void* buf, size_t len)
{
  do {
    // Send.
    const ssize_t ret = ::send(_M_fd, buf, len, MSG_NOSIGNAL);

    // If data has been sent...
    if (ret >= 0) {
      return ret;
    } else {
      if (errno != EINTR) {
        return -1;
      }
    }
  } while (true);
}

bool net::socket::send(const void* buf, size_t len, int timeout)
{
  const uint8_t* b = static_cast<const uint8_t*>(buf);

  // While there is data to be sent...
  while (len > 0) {
    // Send.
    const ssize_t ret = send(b, len);

    // If some data could be sent...
    if (ret >= 0) {
      b += ret;
      len -= ret;
    } else {
      // If the send operation would block...
      if (errno == EAGAIN) {
        // Wait for the socket to be writable.
        if (!wait_writable(timeout)) {
          return false;
        }
      } else {
        return false;
      }
    }
  }

  return true;
}

bool net::socket::wait_readable(int timeout)
{
  struct pollfd fd;
  fd.fd = _M_fd;
  fd.events = POLLIN | POLLRDHUP;

  switch (poll(&fd, 1, timeout)) {
    case 1:
      return true;
    case 0: // Timeout.
      errno = ETIMEDOUT;

      // Fall through.
    default:
      return false;
  }
}

bool net::socket::wait_writable(int timeout)
{
  struct pollfd fd;
  fd.fd = _M_fd;
  fd.events = POLLOUT;

  switch (poll(&fd, 1, timeout)) {
    case 1:
      return true;
    case 0: // Timeout.
      errno = ETIMEDOUT;

      // Fall through.
    default:
      return false;
  }
}

bool net::socket::connect(const struct sockaddr& addr, socklen_t addrlen)
{
  // Create non-blocking socket.
  _M_fd = ::socket(addr.sa_family, SOCK_STREAM | SOCK_NONBLOCK, 0);

  // Success?
  if (_M_fd != -1) {
    do {
      // Connect.
      const int ret = ::connect(_M_fd, &addr, addrlen);

      // Success?
      if (ret == 0) {
        return true;
      } else {
        // If the connection couldn't be completed immediately...
        if (errno == EINPROGRESS) {
          return false;
        } else if (errno != EINTR) {
          // Close socket.
          ::close(_M_fd);
          _M_fd = -1;

          return false;
        }
      }
    } while (true);
  }

  return false;
}

bool net::socket::connect(const struct sockaddr& addr,
                          socklen_t addrlen,
                          int timeout)
{
  // Connect.
  if (connect(addr, addrlen)) {
    // Get socket error.
    int error;
    if ((get_socket_error(error)) && (error == 0)) {
      return true;
    } else {
      // Close socket.
      ::close(_M_fd);
      _M_fd = -1;
    }
  } else {
    // If the connection couldn't be completed immediately...
    if (_M_fd != -1) {
      // Wait for the socket to be writable and, in case of success, check that
      // the socket error is 0.
      int error;
      if ((wait_writable(timeout)) &&
          (get_socket_error(error)) &&
          (error == 0)) {
        return true;
      } else {
        // Close socket.
        ::close(_M_fd);
        _M_fd = -1;
      }
    }
  }

  return false;
}

bool net::socket::listen(const sockaddr& addr, socklen_t addrlen)
{
  // Create non-blocking socket.
  _M_fd = ::socket(addr.sa_family, SOCK_STREAM | SOCK_NONBLOCK, 0);

  // Success?
  if (_M_fd != -1) {
    // Reuse address and port and then bind and listen.
    static constexpr const int optval = 1;
    if ((setsockopt(_M_fd,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    &optval,
                    sizeof(int)) == 0) &&
        (setsockopt(_M_fd,
                    SOL_SOCKET,
                    SO_REUSEPORT,
                    &optval,
                    sizeof(int)) == 0) &&
        (::bind(_M_fd, &addr, addrlen) == 0) &&
        (::listen(_M_fd, SOMAXCONN) == 0)) {
      return true;
    }

    // Close socket.
    ::close(_M_fd);
    _M_fd = -1;
  }

  return false;
}

bool net::socket::accept(socket& sock,
                         struct sockaddr* addr,
                         socklen_t* addrlen)
{
  do {
    // Accept new connection.
    const int fd = ::accept4(_M_fd, addr, addrlen, SOCK_NONBLOCK);

    // Success?
    if (fd != -1) {
      sock._M_fd = fd;
      return true;
    } else {
      if (errno != EINTR) {
        return false;
      }
    }
  } while (true);
}

bool net::socket::accept(socket& sock,
                         struct sockaddr* addr,
                         socklen_t* addrlen,
                         int timeout)
{
  do {
    // Accept new connection.
    if (accept(sock, addr, addrlen)) {
      return true;
    } else {
      // If no connections are present to be accepted...
      if (errno == EAGAIN) {
        return ((wait_readable(timeout)) && (accept(sock, addr, addrlen)));
      } else {
        return false;
      }
    }
  } while (true);
}
