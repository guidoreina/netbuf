#ifndef NET_BUFFER_H
#define NET_BUFFER_H

#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>

namespace net {
  // Forward declaration.
  class buffer;

  // Network buffer.
  class buffer {
    public:
      // Network buffer allocator.
      class allocator {
        public:
          // Constructor.
          allocator() = default;

          // Destructor.
          ~allocator();

          // Get buffer.
          buffer* get();

          // Put buffer.
          void put(buffer* buf);

        private:
          // Buffer allocation.
          static constexpr const size_t allocation = 10000;

          // Pointer to the first buffer.
          buffer* _M_first = nullptr;

          // Mutex.
          pthread_mutex_t _M_mutex = PTHREAD_MUTEX_INITIALIZER;

          // Allocate buffers.
          bool allocate();

          // Disable copy constructor and assignment operator.
          allocator(const allocator&) = delete;
          allocator& operator=(const allocator&) = delete;
      };

      // Constructor.
      buffer() = default;

      // Destructor.
      ~buffer();

      // Clear buffer.
      void clear();

      // Initialize.
      bool init(const void* data, size_t len);

      // Save to file.
      bool save(const char* filename) const;

      // Get data.
      const void* data() const;

      // Get data length.
      size_t length() const;

      // Get previous buffer.
      const buffer* prev() const;
      buffer* prev();

      // Set previous buffer.
      void prev(buffer* buf);

      // Get next buffer.
      const buffer* next() const;
      buffer* next();

      // Set next buffer.
      void next(buffer* buf);

    private:
      // Data.
      void* _M_data = nullptr;

      // Data length.
      size_t _M_length;

      // Previous buffer.
      buffer* _M_prev;

      // Next buffer.
      buffer* _M_next;

      // Disable copy constructor and assignment operator.
      buffer(const buffer&) = delete;
      buffer& operator=(const buffer&) = delete;
  };

  inline buffer::~buffer()
  {
    clear();
  }

  inline void buffer::clear()
  {
    if (_M_data) {
      free(_M_data);
      _M_data = nullptr;
    }
  }

  inline const void* buffer::data() const
  {
    return _M_data;
  }

  inline size_t buffer::length() const
  {
    return _M_length;
  }

  inline const buffer* buffer::prev() const
  {
    return _M_prev;
  }

  inline buffer* buffer::prev()
  {
    return _M_prev;
  }

  inline void buffer::prev(buffer* buf)
  {
    _M_prev = buf;
  }

  inline const buffer* buffer::next() const
  {
    return _M_next;
  }

  inline buffer* buffer::next()
  {
    return _M_next;
  }

  inline void buffer::next(buffer* buf)
  {
    _M_next = buf;
  }
}

#endif // NET_BUFFER_H
