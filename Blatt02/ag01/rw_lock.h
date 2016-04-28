#ifndef RW_LOCK_H
#define RW_LOCK_H

#include <mutex>

class rw_lock {
 private:
  std::mutex reader;
  std::mutex global;
  int blocking_readers = 0;

 public:
  void lock_read();
  void unlock_read();
  void lock_write();
  void unlock_write();
};

struct read_lock_guard {
 private:
  rw_lock& lock;

 public:
  read_lock_guard(rw_lock& lock);
  ~read_lock_guard();
};

struct write_lock_guard {
 private:
  rw_lock& lock;

 public:
  write_lock_guard(rw_lock& lock);
  ~write_lock_guard();
};

#endif /*RW_LOCK_H*/