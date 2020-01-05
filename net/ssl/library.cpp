#include <openssl/err.h>
#include "net/ssl/library.h"
#include "net/ssl/socket.h"

net::ssl::library::~library()
{
  if (_M_ctx) {
    SSL_CTX_free(_M_ctx);
  }
}

bool net::ssl::library::init()
{
  const SSL_METHOD* const method = TLS_method();

  if (method) {
    // Create SSL_CTX object.
    if ((_M_ctx = SSL_CTX_new(method)) != nullptr) {
      ssl::socket::context(_M_ctx);
      return true;
    }
  }

  return false;
}

bool net::ssl::library::load_certificate(const char* filename)
{
  // Clear the error queue.
  ERR_clear_error();

  return (SSL_CTX_use_certificate_chain_file(_M_ctx, filename) == 1);
}

bool net::ssl::library::load_private_key(const char* filename)
{
  // Clear the error queue.
  ERR_clear_error();

  return (SSL_CTX_use_PrivateKey_file(_M_ctx, filename, SSL_FILETYPE_PEM) == 1);
}
