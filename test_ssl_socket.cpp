#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "net/ssl/library.h"
#include "net/ssl/socket.h"

static void usage(const char* program);
static int server(const net::socket::address& addr);
static int client(const net::socket::address& addr);

int main(int argc, const char** argv)
{
  if (argc == 3) {
    if (strcasecmp(argv[1], "--server") == 0) {
      net::socket::address addr;
      if (addr.build(argv[2])) {
        return server(addr);
      } else {
        fprintf(stderr, "Invalid address '%s'.\n", argv[2]);
      }
    } else if (strcasecmp(argv[1], "--client") == 0) {
      net::socket::address addr;
      if (addr.build(argv[2])) {
        return client(addr);
      } else {
        fprintf(stderr, "Invalid address '%s'.\n", argv[2]);
      }
    } else {
      usage(argv[0]);
    }
  } else {
    usage(argv[0]);
  }

  return -1;
}

void usage(const char* program)
{
  fprintf(stderr, "Usage: %s --server|--client <address>\n", program);
}

int server(const net::socket::address& addr)
{
  // Initialize OpenSSL.
  net::ssl::library openssl;
  if (openssl.init()) {
    // Load certificate.
    static constexpr const char* const certificate = "cert.pem";
    if (openssl.load_certificate(certificate)) {
      // Load private key.
      static constexpr const char* const private_key = "key.pem";
      if (openssl.load_private_key(private_key)) {
        char addrstr[INET6_ADDRSTRLEN + 8];
        addr.to_string(addrstr, sizeof(addrstr));

        // Listen.
        net::socket sock;
        if (sock.listen(addr)) {
          printf("Listening on '%s'.\n", addrstr);

          // Accept new connection.
          net::socket client;
          net::socket::address clientaddr;
          if (sock.accept(client, clientaddr, net::socket::default_timeout)) {
            clientaddr.to_string(addrstr, sizeof(addrstr));
            printf("Accepted connection from '%s'.\n", addrstr);

            // Perform TLS/SSL handshake.
            net::ssl::socket sslsock(client);
            if (sslsock.handshake(net::ssl::socket::mode::server,
                                  net::socket::default_timeout)) {
              // Receive.
              char buf[1024];
              const ssize_t ret = sslsock.recv(buf,
                                               sizeof(buf),
                                               net::socket::default_timeout);

              // If some data has been received...
              if (ret > 0) {
                printf("Received %zd bytes (%.*s).\n",
                       ret,
                       static_cast<int>(ret),
                       buf);

                // Send same data.
                if (sslsock.send(buf, ret, net::socket::default_timeout)) {
                  printf("%zd bytes sent.\n", ret);
                  return 0;
                } else {
                  fprintf(stderr, "Error sending.\n");
                }
              } else {
                fprintf(stderr, "Error receiving.\n");
              }
            } else {
              fprintf(stderr, "Error performing handshake.\n");
            }
          } else {
            fprintf(stderr, "Error accepting new connection.\n");
          }
        } else {
          fprintf(stderr, "Error listening on '%s'.\n", addrstr);
        }
      } else {
        fprintf(stderr, "Error loading private key from '%s'.\n", private_key);
      }
    } else {
      fprintf(stderr, "Error loading certificate from '%s'.\n", certificate);
    }
  } else {
    fprintf(stderr, "Error initializing OpenSSL.\n");
  }

  return -1;
}

int client(const net::socket::address& addr)
{
  char addrstr[INET6_ADDRSTRLEN + 8];
  addr.to_string(addrstr, sizeof(addrstr));

  // Initialize OpenSSL.
  net::ssl::library openssl;
  if (openssl.init()) {
    // Connect.
    net::ssl::socket sock;
    if (sock.connect(addr, net::socket::default_timeout)) {
      printf("Connected to '%s'.\n", addrstr);

      // Send.
      if (sock.send("test", 4, net::socket::default_timeout)) {
        printf("Sent 4 bytes.\n");

        // Receive.
        char buf[1024];
        const ssize_t ret = sock.recv(buf,
                                      sizeof(buf),
                                      net::socket::default_timeout);

        if (ret > 0) {
          printf("Received %zd bytes (%.*s).\n",
                 ret,
                 static_cast<int>(ret),
                 buf);
          return 0;
        } else {
          fprintf(stderr, "Error receiving.\n");
        }
      } else {
        fprintf(stderr, "Error sending.\n");
      }
    } else {
      fprintf(stderr, "Error connecting to '%s'.\n", addrstr);
    }
  } else {
    fprintf(stderr, "Error initializing OpenSSL.\n");
  }

  return -1;
}
