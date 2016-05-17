#include "rw_lock.h"

void rw_lock::lock_read() {
  reader.lock();
  blocking_readers++;
  if (blocking_readers == 1) {
    global.lock();
  }
  reader.unlock();
}
void rw_lock::unlock_read() {
  reader.lock();
  blocking_readers--;
  if (blocking_readers == 0) {
    global.unlock();
  }
  reader.unlock();
}
void rw_lock::lock_write() { global.lock(); }
void rw_lock::unlock_write() { global.unlock(); }

read_lock_guard::read_lock_guard(rw_lock& lock) : lock(lock) {
  lock.lock_read();
}
read_lock_guard::~read_lock_guard() { lock.unlock_read(); }

write_lock_guard::write_lock_guard(rw_lock& lock) : lock(lock) {
  lock.lock_write();
}
write_lock_guard::~write_lock_guard() { lock.unlock_write(); }
