#include "re_internal.h"

#if RE_USE_THREAD

#if RE__THREAD_PLATFORM == RE__THREADS_POSIX

#include <errno.h>
#include <unistd.h>

int re__mutex_init(re__mutex* mutex)
{
  int code = pthread_mutex_init(mutex, NULL);
  if (code == EAGAIN) {
    return 1;
  } else if (code == ENOMEM) {
    return 1;
  } else if (code) {
    MN_ASSERT(0);
  } else {
    return 0;
  }
  return 0; /* GCC says "control reaches end of non-void function." The fuck? */
}

void re__mutex_destroy(re__mutex* mutex)
{
  int code = pthread_mutex_destroy(mutex);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

void re__mutex_lock(re__mutex* mutex)
{
  int code = pthread_mutex_lock(mutex);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

void re__mutex_unlock(re__mutex* mutex)
{
  int code = pthread_mutex_unlock(mutex);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

int re__rwlock_init(re__rwlock* rwlock)
{
  int code = pthread_rwlock_init(rwlock, NULL);
  if (code == EAGAIN) {
    return 1;
  } else if (code == ENOMEM) {
    return 1;
  } else if (code) {
    MN_ASSERT(0);
  } else {
    return 0;
  }
  return 0;
}

void re__rwlock_destroy(re__rwlock* rwlock)
{
  int code = pthread_rwlock_destroy(rwlock);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

void re__rwlock_rlock(re__rwlock* rwlock)
{
  int code = pthread_rwlock_rdlock(rwlock);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

void re__rwlock_wlock(re__rwlock* rwlock)
{
  int code = pthread_rwlock_wrlock(rwlock);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

void re__rwlock_runlock(re__rwlock* rwlock)
{
  int code = pthread_rwlock_unlock(rwlock);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

void re__rwlock_wunlock(re__rwlock* rwlock)
{
  int code = pthread_rwlock_unlock(rwlock);
  MN_ASSERT(!code);
  MN__UNUSED(code);
}

#endif

#endif /* RE_USE_THREAD */
