#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "net/sender.h"

bool net::sender::start(encryption enc, const char* directory)
{
  const size_t dirlen = strlen(directory);

  // If the directory name is not too long...
  if (dirlen < sizeof(_M_directory)) {
    // If the directory exists...
    struct stat sbuf;
    if ((stat(directory, &sbuf) == 0) && (S_ISDIR(sbuf.st_mode))) {
      // Save directory name.
      memcpy(_M_directory, directory, dirlen);
      _M_directory[dirlen] = 0;

      // Set callbacks.
      if (enc == encryption::no) {
        _M_connect = &sender::connect_insecure;
        _M_disconnect = &sender::disconnect_insecure;
        _M_send = &sender::send_insecure;
        _M_connected = &sender::connected_insecure;

        _M_connection_closed_by_peer =
          &sender::connection_closed_by_peer_insecure;
      } else {
        _M_connect = &sender::connect_secure;
        _M_disconnect = &sender::disconnect_secure;
        _M_send = &sender::send_secure;
        _M_connected = &sender::connected_secure;

        _M_connection_closed_by_peer =
          &sender::connection_closed_by_peer_secure;
      }

      _M_running = true;

      // Start thread.
      if (pthread_create(&_M_thread, nullptr, start_routine, this) == 0) {
        return true;
      }

      _M_running = false;
    }
  }

  return false;
}

void net::sender::stop()
{
  // If the thread is running...
  if (_M_running) {
    _M_running = false;
    pthread_join(_M_thread, nullptr);
  }
}

void net::sender::send(buffer* buf)
{
  if (_M_buffers.push_back(buf) > max_queued_buffers) {
    // Save buffers to disk.
    save_buffers();

    _M_state = state::sending_files;
  }
}

void net::sender::send(buffer* first, buffer* last)
{
  if (_M_buffers.push_back(first, last) > max_queued_buffers) {
    // Save buffers to disk.
    save_buffers();

    _M_state = state::sending_files;
  }
}

void net::sender::run()
{
  // Sleep interval in microseconds.
  static constexpr const useconds_t sleep_interval = 250000;

  // Error sending?
  bool error_sending = false;

  do {
    // Save current time.
    _M_current_time = time(nullptr);

    // If there was an error sending...
    if (error_sending) {
      // If we can try to connect now...
      if (_M_current_time - _M_last_socket_operation >= reconnection_time) {
#if DEBUG
        printf("[sender::run] Will try to connect now.\n");
#endif

        // Connect.
        error_sending = !connect();

#if DEBUG
        printf("[sender::run] %s.\n",
               error_sending ? "Error connecting" : "Connected");
#endif // DEBUG

        // If we couldn't connect...
        if (error_sending) {
          usleep(sleep_interval);
          continue;
        }
      } else {
#if DEBUG
        printf("[sender::run] Won't try to connect now.\n");
#endif

        usleep(sleep_interval);
        continue;
      }
    }

    // If we are sending queued buffers...
    if (_M_state == state::sending_queued_buffers) {
      // Get queued buffers.
      buffer* first;
      buffer* last;
      if (_M_buffers.pop(first, last, sleep_interval / 1000) > 0) {
#if DEBUG
        printf("[sender::run] There are queued buffers to be sent.\n");
#endif

        // Connect (if not already connected).
        if (connect()) {
          // For each queued buffer...
          do {
            // Send queued buffer.
            if (send(first->data(), first->length())) {
#if DEBUG
              printf("[sender::run] Queued buffer sent.\n");
#endif

              // If not the last buffer...
              if (first != last) {
                buffer* next = first->next();

                // Return buffer to the allocator.
                _M_allocator.put(first);

                first = next;
              } else {
                // Last buffer.

                // Return buffer to the allocator.
                _M_allocator.put(first);

                break;
              }
            } else {
#if DEBUG
              printf("[sender::run] Error sending queued buffer.\n");
#endif

              // Return buffers to the buffer pool.
              _M_buffers.push_front(first, last);

              error_sending = true;

              break;
            }
          } while (true);
        } else {
#if DEBUG
          printf("[sender::run] Error connecting.\n");
#endif

          // Return buffers to the buffer pool.
          _M_buffers.push_front(first, last);

          error_sending = true;
        }
      } else {
        // If we are connected and have been idle for a long time...
        if ((connected()) &&
            (_M_current_time - _M_last_socket_operation >= idle_timeout)) {
#if DEBUG
          printf("[sender::run] Disconnecting.\n");
#endif

          // Disconnect.
          disconnect();
#if DEBUG
        } else {
          printf("[sender::run] No buffers to be sent.\n");
#endif // DEBUG
        }
      }
    } else {
      // There might be buffers on disk.

#if DEBUG
      printf("[sender::run] There might be buffers on disk to be sent.\n");
#endif

      // Send buffers on disk.
      error_sending = !send_files();

#if DEBUG
      printf("[sender::run] %s.\n",
             error_sending ?
               "Error sending buffers on disk" :
               "Possible buffers on disk have been successfully sent");
#endif // DEBUG
    }
  } while (_M_running);

  // Save buffers to disk (if any).
  save_buffers();
}

bool net::sender::send(const void* buf, size_t len)
{
  // Save time of the last socket operation.
  _M_last_socket_operation = _M_current_time;

  // Send data if the peer has not closed the connection.
  if ((!connection_closed_by_peer()) && ((this->*_M_send)(buf, len))) {
    return true;
  } else {
    // Disconnect.
    disconnect();

    return false;
  }
}

bool net::sender::send_files()
{
  // Connect (if not already connected).
  if (connect()) {
    // Open directory.
    DIR* dir = opendir(_M_directory);

    // If the directory could be opened...
    if (dir) {
      // For each directory entry...
      struct dirent* entry;
      while ((entry = readdir(dir)) != nullptr) {
        // If neither the current directory nor the parent directory...
        if ((strcmp(entry->d_name, ".") != 0) &&
            (strcmp(entry->d_name, "..") != 0)) {
          // Compose filename.
          char filename[PATH_MAX + 256];
          snprintf(filename,
                   sizeof(filename),
                   "%s/%s",
                   _M_directory,
                   entry->d_name);

          // If the file exists and is a regular file...
          struct stat sbuf;
          if ((stat(filename, &sbuf) == 0) && (S_ISREG(sbuf.st_mode))) {
            // Send file.
            if (send_file(filename, sbuf.st_size)) {
              // Remove file.
              unlink(filename);
            } else {
              closedir(dir);

              return false;
            }
          }
        }
      }

      closedir(dir);

      _M_state = state::sending_queued_buffers;

      return true;
    }
  }

  return false;
}

bool net::sender::send_file(const char* filename, size_t filesize)
{
  // Open file for reading.
  const int fd = open(filename, O_RDONLY);

  // If the file could be opened...
  if (fd != -1) {
    // Map file into memory.
    void* base = mmap(nullptr, filesize, PROT_READ, MAP_SHARED, fd, 0);

    // If the file could be mapped into memory...
    if (base != MAP_FAILED) {
      // Send file content.
      const bool ret = send(base, filesize);

      munmap(base, filesize);
      close(fd);

      return ret;
    }

    close(fd);
  }

  // Ignore file.
  return true;
}

bool net::sender::save_buffers()
{
  bool ret = true;

  unsigned count = 0u;

  // Get queued buffers.
  buffer* first;
  buffer* last;
  if (_M_buffers.pop(first, last) > 0) {
    // For each queued buffer...
    do {
      // Save buffer to disk.
      if (save_buffer(first, count)) {
        // If not the last buffer...
        if (first != last) {
          buffer* next = first->next();

          // Return buffer to the allocator.
          _M_allocator.put(first);

          first = next;
        } else {
          // Last buffer.

          // Return buffer to the allocator.
          _M_allocator.put(first);

          break;
        }
      } else {
        // Return buffer to the allocator.
        _M_allocator.put(first);

        ret = false;
      }
    } while (true);
  }

  return ret;
}

bool net::sender::save_buffer(const buffer* buf, unsigned& count)
{
  struct tm tm;
  localtime_r(&_M_current_time, &tm);

  // Work with a copy of 'count'.
  unsigned cnt = count;

  char filename[PATH_MAX + 23];
  struct stat sbuf;

  // Search a filename which is not in use.
  do {
    snprintf(filename,
             sizeof(filename),
             "%s/%04u%02u%02u-%02u%02u%02u_%06u",
             _M_directory,
             1900 + tm.tm_year,
             1 + tm.tm_mon,
             tm.tm_mday,
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec,
             cnt++);
  } while (stat(filename, &sbuf) == 0);

  count = cnt;

  return buf->save(filename);
}
