#ifndef NET_SSL_LIBRARY_H
#define NET_SSL_LIBRARY_H

#include <openssl/ssl.h>

namespace net {
  namespace ssl {
    class library {
      public:
        // Constructor.
        library() = default;

        // Destructor.
        ~library();

        // Initialize.
        bool init();

        // Load certificate.
        bool load_certificate(const char* filename);

        // Load private key.
        bool load_private_key(const char* filename);

      private:
        // Context object.
        SSL_CTX* _M_ctx = nullptr;

        // Disable copy constructor and assignment operator.
        library(const library&) = delete;
        library& operator=(const library&) = delete;
    };
  }
}

#endif // NET_SSL_LIBRARY_H
