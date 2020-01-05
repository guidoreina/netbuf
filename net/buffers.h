#ifndef NET_BUFFERS_H
#define NET_BUFFERS_H

#include "net/buffer.h"

namespace net {
  // Network buffers.
  class buffers {
    public:
      // Constructor.
      buffers();

      // Destructor.
      ~buffers();

      // Add buffer to the front.
      size_t push_front(buffer* buf);

      // Add buffers to the front.
      size_t push_front(buffer* first, buffer* last);

      // Add buffer to the end.
      size_t push_back(buffer* buf);

      // Add buffers to the end.
      size_t push_back(buffer* first, buffer* last);

      // Get first buffer.
      buffer* pop_front();

      // Get first buffer (timeout in milliseconds).
      buffer* pop_front(unsigned timeout);

      // Get all buffers.
      size_t pop(buffer*& first, buffer*& last);

      // Get all buffers (timeout in milliseconds).
      size_t pop(buffer*& first, buffer*& last, unsigned timeout);

    private:
      // Header.
      buffer _M_header;

      // Number of buffers.
      size_t _M_count = 0;

      // Mutex.
      pthread_mutex_t _M_mutex = PTHREAD_MUTEX_INITIALIZER;

      // Condition variable.
      pthread_cond_t _M_cond = PTHREAD_COND_INITIALIZER;

      // Compute number of buffers in the list.
      static size_t distance(const buffer* first, const buffer* last);

      // Disable copy constructor and assignment operator.
      buffers(const buffers&) = delete;
      buffers& operator=(const buffers&) = delete;
  };

  inline buffers::buffers()
  {
    _M_header.prev(&_M_header);
    _M_header.next(&_M_header);
  }

  inline size_t buffers::distance(const buffer* first, const buffer* last)
  {
    size_t count = 1;

    while (first != last) {
      first = first->next();
      count++;
    }

    return count;
  }
}

#endif // NET_BUFFERS_H
