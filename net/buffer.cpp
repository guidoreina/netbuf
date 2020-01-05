#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <new>
#include "net/buffer.h"

net::buffer::allocator::~allocator()
{
  buffer* buf = _M_first;
  while (buf) {
    buffer* next = buf->next();

    delete buf;

    buf = next;
  }

  pthread_mutex_destroy(&_M_mutex);
}

net::buffer* net::buffer::allocator::get()
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  if ((_M_first) || (allocate())) {
    buffer* buf = _M_first;

    _M_first = _M_first->next();

    // Unlock mutex.
    pthread_mutex_unlock(&_M_mutex);

    return buf;
  }

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);

  return nullptr;
}

void net::buffer::allocator::put(buffer* buf)
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  buf->next(_M_first);

  _M_first = buf;

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);
}

bool net::buffer::allocator::allocate()
{
  for (size_t i = allocation; i > 0; i--) {
    buffer* buf = new (std::nothrow) buffer();

    if (buf) {
      buf->next(_M_first);

      _M_first = buf;
    } else {
      break;
    }
  }

  return (_M_first != nullptr);
}

bool net::buffer::init(const void* data, size_t len)
{
  void* buf = realloc(_M_data, len);
  if (buf) {
    memcpy(buf, data, len);

    _M_data = buf;
    _M_length = len;

    return true;
  }

  return false;
}

bool net::buffer::save(const char* filename) const
{
  // Open file for writing.
  const int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

  // If the file could be opened...
  if (fd != -1) {
    const uint8_t* data = static_cast<const uint8_t*>(_M_data);

    size_t written = 0;
    while (written < _M_length) {
      const ssize_t ret = write(fd, data, _M_length - written);

      if (ret > 0) {
        data += ret;
        written += ret;
      } else if (ret < 0) {
        if (errno != EINTR) {
          close(fd);
          unlink(filename);

          return false;
        }
      }
    }

    close(fd);

    return true;
  }

  return false;
}
