#include <errno.h>
#include <openssl/err.h>
#include "net/ssl/socket.h"

SSL_CTX* net::ssl::socket::_M_ctx = nullptr;

bool net::ssl::socket::shutdown(shutdown_how how, int timeout)
{
  // If the shutdown is unidirectional...
  if (how == shutdown_how::unidirectional) {
    SSL_set_shutdown(_M_ssl, SSL_get_shutdown(_M_ssl) | SSL_RECEIVED_SHUTDOWN);
  }

  do {
    // Reset errno.
    errno = 0;

    // Clear the error queue.
    ERR_clear_error();

    // Shutdown the TLS/SSL connection.
    const int ret = SSL_shutdown(_M_ssl);

    switch (ret) {
      case 1:
        return true;
      case 0:
        {
          // Discard data sent by the peer.
          do {
            uint8_t buf[BUFSIZ];
            switch (recv(buf, sizeof(buf), timeout)) {
              default:
                break;
              case 0:
                return true;
              case -1:
                return false;
            }
          } while (true);
        }

        break;
      default:
        switch (SSL_get_error(_M_ssl, ret)) {
          case SSL_ERROR_ZERO_RETURN:
            return true;
          case SSL_ERROR_WANT_READ:
            // Wait for the socket to be readable.
            if (!_M_sock.wait_readable(timeout)) {
              return false;
            }

            break;
          case SSL_ERROR_WANT_WRITE:
            // Wait for the socket to be writable.
            if (!_M_sock.wait_writable(timeout)) {
              return false;
            }

            break;
          case SSL_ERROR_SYSCALL:
            if (errno != EINTR) {
              return false;
            }

            break;
          case SSL_ERROR_SSL:
          default:
            return false;
        }
    }
  } while (true);
}

bool net::ssl::socket::handshake(mode m, int timeout)
{
  // Create a new SSL structure.
  if ((_M_ssl = SSL_new(_M_ctx)) != nullptr) {
    if (SSL_set_fd(_M_ssl, _M_sock.fd())) {
      if (m == mode::client) {
        SSL_set_connect_state(_M_ssl);
      } else {
        SSL_set_accept_state(_M_ssl);
      }

      // Perform TLS/SSL handshake.
      if (handshake(timeout)) {
        return true;
      }
    }

    SSL_free(_M_ssl);
    _M_ssl = nullptr;
  }

  return false;
}

ssize_t net::ssl::socket::recv(void* buf, size_t len)
{
  do {
    // Reset errno.
    errno = 0;

    // Clear the error queue.
    ERR_clear_error();

    // Receive.
    const int ret = SSL_read(_M_ssl, buf, len);

    // Success?
    if (ret > 0) {
      return ret;
    } else {
      switch (SSL_get_error(_M_ssl, ret)) {
        case SSL_ERROR_ZERO_RETURN:
          return 0;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          errno = EAGAIN;
          return -1;
        case SSL_ERROR_SYSCALL:
          if (errno != EINTR) {
            return -1;
          }

          break;
        case SSL_ERROR_SSL:
        default:
          return -1;
      }
    }
  } while (true);
}

ssize_t net::ssl::socket::recv(void* buf, size_t len, int timeout)
{
  do {
    // Reset errno.
    errno = 0;

    // Clear the error queue.
    ERR_clear_error();

    // Receive.
    const int ret = SSL_read(_M_ssl, buf, len);

    // Success?
    if (ret > 0) {
      return ret;
    } else {
      switch (SSL_get_error(_M_ssl, ret)) {
        case SSL_ERROR_ZERO_RETURN:
          return 0;
        case SSL_ERROR_WANT_READ:
          // Wait for the socket to be readable.
          if (!_M_sock.wait_readable(timeout)) {
            return -1;
          }

          break;
        case SSL_ERROR_WANT_WRITE:
          // Wait for the socket to be writable.
          if (!_M_sock.wait_writable(timeout)) {
            return -1;
          }

          break;
        case SSL_ERROR_SYSCALL:
          if (errno != EINTR) {
            return -1;
          }

          break;
        case SSL_ERROR_SSL:
        default:
          return -1;
      }
    }
  } while (true);
}

bool net::ssl::socket::send(const void* buf, size_t len)
{
  do {
    // Reset errno.
    errno = 0;

    // Clear the error queue.
    ERR_clear_error();

    // Send.
    const int ret = SSL_write(_M_ssl, buf, len);

    // Success?
    if (ret > 0) {
      return true;
    } else {
      switch (SSL_get_error(_M_ssl, ret)) {
        case SSL_ERROR_ZERO_RETURN:
          return false;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          errno = EAGAIN;
          return false;
        case SSL_ERROR_SYSCALL:
          if (errno != EINTR) {
            return false;
          }

          break;
        case SSL_ERROR_SSL:
        default:
          return false;
      }
    }
  } while (true);
}

bool net::ssl::socket::send(const void* buf, size_t len, int timeout)
{
  do {
    // Reset errno.
    errno = 0;

    // Clear the error queue.
    ERR_clear_error();

    // Send.
    const int ret = SSL_write(_M_ssl, buf, len);

    // Success?
    if (ret > 0) {
      return true;
    } else {
      switch (SSL_get_error(_M_ssl, ret)) {
        case SSL_ERROR_ZERO_RETURN:
          return false;
        case SSL_ERROR_WANT_READ:
          // Wait for the socket to be readable.
          if (!_M_sock.wait_readable(timeout)) {
            return false;
          }

          break;
        case SSL_ERROR_WANT_WRITE:
          // Wait for the socket to be writable.
          if (!_M_sock.wait_writable(timeout)) {
            return false;
          }

          break;
        case SSL_ERROR_SYSCALL:
          if (errno != EINTR) {
            return false;
          }

          break;
        case SSL_ERROR_SSL:
        default:
          return false;
      }
    }
  } while (true);
}

bool net::ssl::socket::connect(int timeout)
{
  // Perform handshake.
  if (handshake(mode::client, timeout)) {
    return true;
  }

  // Close socket.
  _M_sock.close();

  return false;
}

bool net::ssl::socket::handshake(int timeout)
{
  do {
    // Reset errno.
    errno = 0;

    // Clear the error queue.
    ERR_clear_error();

    // Perform TLS/SSL handshake.
    const int ret = SSL_do_handshake(_M_ssl);

    switch (ret) {
      case 1:
        return true;
      case 0:
        return false;
      default:
        switch (SSL_get_error(_M_ssl, ret)) {
          case SSL_ERROR_ZERO_RETURN:
            return false;
          case SSL_ERROR_WANT_READ:
            // Wait for the socket to be readable.
            if (!_M_sock.wait_readable(timeout)) {
              return false;
            }

            break;
          case SSL_ERROR_WANT_WRITE:
            // Wait for the socket to be writable.
            if (!_M_sock.wait_writable(timeout)) {
              return false;
            }

            break;
          case SSL_ERROR_SYSCALL:
            if (errno != EINTR) {
              return false;
            }

            break;
          case SSL_ERROR_SSL:
          default:
            return false;
        }
    }
  } while (true);
}
