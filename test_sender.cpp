#include <stdlib.h>
#include <stdio.h>
#include "net/sender.h"

int main(int argc, const char** argv)
{
  if (argc == 2) {
    // Buffer allocator.
    net::buffer::allocator allocator;

    // Start sender.
    net::sender sender(allocator);
    if (sender.start(argv[1], net::sender::encryption::no, "buffersdir")) {
      unsigned count = 0;

      do {
        printf("Press 'q' to quit.\n");

        // Get character.
        int c = getchar();

        if (c != 'q') {
          // Get a free buffer.
          net::buffer* buf = allocator.get();
          if (buf) {
            char s[256];
            size_t len = snprintf(s, sizeof(s), "%06u\n", count++);

            // Initialize buffer.
            if (buf->init(s, len)) {
              // Send buffer.
              sender.send(buf);
            } else {
              fprintf(stderr, "Error initializing buffer.\n");

              allocator.put(buf);

              return -1;
            }
          } else {
            fprintf(stderr, "Error getting free buffer.\n");
            return -1;
          }
        } else {
          break;
        }
      } while (true);

      return 0;
    } else {
      fprintf(stderr, "Error starting sender.\n");
    }
  } else {
    fprintf(stderr, "Usage: %s <address>\n", argv[0]);
  }

  return -1;
}
