#include <sys/time.h>
#include "net/buffers.h"

net::buffers::~buffers()
{
  buffer* buf = _M_header.next();
  while (buf != &_M_header) {
    buffer* next = buf->next();

    delete buf;

    buf = next;
  }

  pthread_cond_destroy(&_M_cond);
  pthread_mutex_destroy(&_M_mutex);
}

size_t net::buffers::push_front(buffer* buf)
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  buf->prev(&_M_header);
  buf->next(_M_header.next());

  _M_header.next()->prev(buf);
  _M_header.next(buf);

  // Increment number of buffers.
  const size_t count = ++_M_count;

  // Restart one of the threads waiting on the condition variable.
  pthread_cond_signal(&_M_cond);

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);

  return count;
}

size_t net::buffers::push_front(buffer* first, buffer* last)
{
  // Compute number of buffers to be added.
  size_t count = distance(first, last);

  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  first->prev(&_M_header);
  last->next(_M_header.next());

  _M_header.next()->prev(last);
  _M_header.next(first);

  // Increment number of buffers.
  _M_count += count;

  count = _M_count;

  // Restart one of the threads waiting on the condition variable.
  pthread_cond_signal(&_M_cond);

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);

  return count;
}

size_t net::buffers::push_back(buffer* buf)
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  buf->prev(_M_header.prev());
  buf->next(&_M_header);

  _M_header.prev()->next(buf);
  _M_header.prev(buf);

  // Increment number of buffers.
  const size_t count = ++_M_count;

  // Restart one of the threads waiting on the condition variable.
  pthread_cond_signal(&_M_cond);

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);

  return count;
}

size_t net::buffers::push_back(buffer* first, buffer* last)
{
  // Compute number of buffers to be added.
  size_t count = distance(first, last);

  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  first->prev(_M_header.prev());
  last->next(&_M_header);

  _M_header.prev()->next(first);
  _M_header.prev(last);

  // Increment number of buffers.
  _M_count += count;

  count = _M_count;

  // Restart one of the threads waiting on the condition variable.
  pthread_cond_signal(&_M_cond);

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);

  return count;
}

net::buffer* net::buffers::pop_front()
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  // Get first buffer (if any).
  buffer* buf = _M_header.next();

  // If there is at least one buffer...
  if (buf != &_M_header) {
    buf->next()->prev(&_M_header);
    _M_header.next(buf->next());

    // Decrement number of buffers.
    _M_count--;
  } else {
    buf = nullptr;
  }

  // Unlock mutex.
  pthread_mutex_unlock(&_M_mutex);

  return buf;
}

net::buffer* net::buffers::pop_front(unsigned timeout)
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  // Get first buffer (if any).
  buffer* buf = _M_header.next();

  // If there is at least one buffer...
  if (buf != &_M_header) {
    buf->next()->prev(&_M_header);
    _M_header.next(buf->next());

    // Decrement number of buffers.
    _M_count--;

    // Unlock mutex.
    pthread_mutex_unlock(&_M_mutex);

    return buf;
  }

  // Get current time.
  struct timeval now;
  gettimeofday(&now, nullptr);

  // Compute absolute time.
  struct timespec ts;
  ts.tv_sec = now.tv_sec + (timeout / 1000);
  ts.tv_nsec = (now.tv_usec * 1000) + ((timeout % 1000) * 1000000);

  if (ts.tv_nsec >= 1000000000l) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000l;
  }

  do {
    if (pthread_cond_timedwait(&_M_cond, &_M_mutex, &ts) == 0) {
      if ((buf = _M_header.next()) != &_M_header) {
        buf->next()->prev(&_M_header);
        _M_header.next(buf->next());

        // Decrement number of buffers.
        _M_count--;

        // Unlock mutex.
        pthread_mutex_unlock(&_M_mutex);

        return buf;
      }
    } else {
      // Unlock mutex.
      pthread_mutex_unlock(&_M_mutex);

      return nullptr;
    }
  } while (true);
}

size_t net::buffers::pop(buffer*& first, buffer*& last)
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  // If there is at least one buffer...
  if (_M_header.next() != &_M_header) {
    first = _M_header.next();
    last = _M_header.prev();

    const size_t count = _M_count;

    _M_header.prev(&_M_header);
    _M_header.next(&_M_header);

    _M_count = 0;

    // Unlock mutex.
    pthread_mutex_unlock(&_M_mutex);

    return count;
  } else {
    // Unlock mutex.
    pthread_mutex_unlock(&_M_mutex);

    first = nullptr;
    last = nullptr;

    return 0;
  }
}

size_t net::buffers::pop(buffer*& first, buffer*& last, unsigned timeout)
{
  // Lock mutex.
  pthread_mutex_lock(&_M_mutex);

  // If there is at least one buffer...
  if (_M_header.next() != &_M_header) {
    first = _M_header.next();
    last = _M_header.prev();

    const size_t count = _M_count;

    _M_header.prev(&_M_header);
    _M_header.next(&_M_header);

    _M_count = 0;

    // Unlock mutex.
    pthread_mutex_unlock(&_M_mutex);

    return count;
  }

  // Get current time.
  struct timeval now;
  gettimeofday(&now, nullptr);

  // Compute absolute time.
  struct timespec ts;
  ts.tv_sec = now.tv_sec + (timeout / 1000);
  ts.tv_nsec = (now.tv_usec * 1000) + ((timeout % 1000) * 1000000);

  if (ts.tv_nsec >= 1000000000l) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000l;
  }

  do {
    if (pthread_cond_timedwait(&_M_cond, &_M_mutex, &ts) == 0) {
      // If there is at least one buffer...
      if (_M_header.next() != &_M_header) {
        first = _M_header.next();
        last = _M_header.prev();

        const size_t count = _M_count;

        _M_header.prev(&_M_header);
        _M_header.next(&_M_header);

        _M_count = 0;

        // Unlock mutex.
        pthread_mutex_unlock(&_M_mutex);

        return count;
      }
    } else {
      // Unlock mutex.
      pthread_mutex_unlock(&_M_mutex);

      first = nullptr;
      last = nullptr;

      return 0;
    }
  } while (true);
}
