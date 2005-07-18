/* Test of locking in multithreaded situations.
   Copyright (C) 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2005.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if USE_POSIX_THREADS || USE_SOLARIS_THREADS || USE_PTH_THREADS || USE_WIN32_THREADS

#if USE_POSIX_THREADS
# define TEST_POSIX_THREADS 1
#endif
#if USE_SOLARIS_THREADS
# define TEST_SOLARIS_THREADS 1
#endif
#if USE_PTH_THREADS
# define TEST_PTH_THREADS 1
#endif
#if USE_WIN32_THREADS
# define TEST_WIN32_THREADS 1
#endif

/* Whether to enable locking.
   Uncomment this to get a test program without locking, to verify that
   it crashes.  */
#define ENABLE_LOCKING 1

/* Which tests to perform.
   Uncomment some of these, to verify that all tests crash if no locking
   is enabled.  */
#define DO_TEST_LOCK 1
#define DO_TEST_RWLOCK 1
#define DO_TEST_RECURSIVE_LOCK 1

/* Whether to help the scheduler through explicit yield().
   Uncomment this to see if the operating system has a fair scheduler.  */
#define EXPLICIT_YIELD 1

/* Whether to print debugging messages.  */
#define ENABLE_DEBUGGING 0

/* Number of simultaneous threads.  */
#define THREAD_COUNT 10

/* Number of operations performed in each thread.
   This is quite high, because with a smaller count, say 5000, we often get
   an "OK" result even without ENABLE_LOCKING (on Linux/x86).  */
#define REPEAT_COUNT 50000

#include <stdio.h>
#include <stdlib.h>

#if !ENABLE_LOCKING
# undef USE_POSIX_THREADS
# undef USE_SOLARIS_THREADS
# undef USE_PTH_THREADS
# undef USE_WIN32_THREADS
#endif
#include "lock.h"

#if ENABLE_DEBUGGING
# define dbgprintf printf
#else
# define dbgprintf if (0) printf
#endif

#if TEST_POSIX_THREADS
# include <pthread.h>
# include <sched.h>
typedef pthread_t gl_thread_t;
static inline gl_thread_t gl_thread_create (void * (*func) (void *), void *arg)
{
  pthread_t thread;
  if (pthread_create (&thread, NULL, func, arg) != 0)
    abort ();
  return thread;
}
static inline void gl_thread_join (gl_thread_t thread)
{
  void *retval;
  if (pthread_join (thread, &retval) != 0)
    abort ();
}
static inline void gl_thread_yield (void)
{
  sched_yield ();
}
static inline void * gl_thread_self (void)
{
  return (void *) pthread_self ();
}
#endif
#if TEST_PTH_THREADS
# include <pth.h>
typedef pth_t gl_thread_t;
static inline gl_thread_t gl_thread_create (void * (*func) (void *), void *arg)
{
  pth_t thread = pth_spawn (NULL, func, arg);
  if (thread == NULL)
    abort ();
  return thread;
}
static inline void gl_thread_join (gl_thread_t thread)
{
  if (!pth_join (thread, NULL))
    abort ();
}
static inline void gl_thread_yield (void)
{
  pth_yield (NULL);
}
static inline void * gl_thread_self (void)
{
  return pth_self ();
}
#endif
#if TEST_SOLARIS_THREADS
# include <thread.h>
typedef thread_t gl_thread_t;
static inline gl_thread_t gl_thread_create (void * (*func) (void *), void *arg)
{
  thread_t thread;
  if (thr_create (NULL, 0, func, arg, 0, &thread) != 0)
    abort ();
  return thread;
}
static inline void gl_thread_join (gl_thread_t thread)
{
  void *retval;
  if (thr_join (thread, NULL, &retval) != 0)
    abort ();
}
static inline void gl_thread_yield (void)
{
  thr_yield ();
}
static inline void * gl_thread_self (void)
{
  return (void *) thr_self ();
}
#endif
#if TEST_WIN32_THREADS
# include <windows.h>
typedef HANDLE gl_thread_t;
/* Use a wrapper function, instead of adding WINAPI through a cast.  */
struct wrapper_args { void * (*func) (void *); void *arg; };
static DWORD WINAPI wrapper_func (void *varg)
{
  struct wrapper_args *warg = (struct wrapper_args *)varg;
  void * (*func) (void *) = warg->func;
  void *arg = warg->arg;
  free (warg);
  func (arg);
  return 0;
}
static inline gl_thread_t gl_thread_create (void * (*func) (void *), void *arg)
{
  struct wrapper_args *warg =
    (struct wrapper_args *) malloc (sizeof (struct wrapper_args));
  if (warg == NULL)
    abort ();
  warg->func = func;
  warg->arg = arg;
  {
    DWORD thread_id;
    HANDLE thread =
      CreateThread (NULL, 100000, wrapper_func, warg, 0, &thread_id);
    if (thread == NULL)
      abort ();
    return thread;
  }
}
static inline void gl_thread_join (gl_thread_t thread)
{
  if (WaitForSingleObject (thread, INFINITE) == WAIT_FAILED)
    abort ();
  if (!CloseHandle (thread))
    abort ();
}
static inline void gl_thread_yield (void)
{
  Sleep (0);
}
static inline void * gl_thread_self (void)
{
  return (void *) GetCurrentThreadId ();
}
#endif
#if EXPLICIT_YIELD
# define yield() gl_thread_yield ()
#else
# define yield()
#endif

#define ACCOUNT_COUNT 4

static int account[ACCOUNT_COUNT];

static int
random_account (void)
{
  return ((unsigned int) rand() >> 3) % ACCOUNT_COUNT;
}

static void
check_accounts (void)
{
  int i, sum;

  sum = 0;
  for (i = 0; i < ACCOUNT_COUNT; i++)
    sum += account[i];
  if (sum != ACCOUNT_COUNT * 1000)
    abort ();
}

/* Test normal locks by having several bank accounts and several threads
   which shuffle around money between the accounts and another thread
   checking that all the money is still there.  */

gl_lock_define_initialized(static, my_lock)

static void *
lock_mutator_thread (void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      int i1, i2, value;

      dbgprintf ("Mutator %p before lock\n", gl_thread_self ());
      gl_lock_lock (my_lock);
      dbgprintf ("Mutator %p after  lock\n", gl_thread_self ());

      i1 = random_account ();
      i2 = random_account ();
      value = ((unsigned int) rand() >> 3) % 10;
      account[i1] += value;
      account[i2] -= value;

      dbgprintf ("Mutator %p before unlock\n", gl_thread_self ());
      gl_lock_unlock (my_lock);
      dbgprintf ("Mutator %p after  unlock\n", gl_thread_self ());

      dbgprintf ("Mutator %p before check lock\n", gl_thread_self ());
      gl_lock_lock (my_lock);
      check_accounts ();
      gl_lock_unlock (my_lock);
      dbgprintf ("Mutator %p after  check unlock\n", gl_thread_self ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self ());
  return NULL;
}

static volatile int lock_checker_done;

static void *
lock_checker_thread (void *arg)
{
  while (!lock_checker_done)
    {
      dbgprintf ("Checker %p before check lock\n", gl_thread_self ());
      gl_lock_lock (my_lock);
      check_accounts ();
      gl_lock_unlock (my_lock);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self ());
  return NULL;
}

void
test_lock (void)
{
  int i;
  gl_thread_t checkerthread;
  gl_thread_t threads[THREAD_COUNT];

  /* Initialization.  */
  for (i = 0; i < ACCOUNT_COUNT; i++)
    account[i] = 1000;
  lock_checker_done = 0;

  /* Spawn the threads.  */
  checkerthread = gl_thread_create (lock_checker_thread, NULL);
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] = gl_thread_create (lock_mutator_thread, NULL);

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i]);
  lock_checker_done = 1;
  gl_thread_join (checkerthread);
  check_accounts ();
}

/* Test read-write locks by having several bank accounts and several threads
   which shuffle around money between the accounts and several other threads
   that check that all the money is still there.  */

gl_rwlock_define_initialized(static, my_rwlock)

static void *
rwlock_mutator_thread (void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      int i1, i2, value;

      dbgprintf ("Mutator %p before wrlock\n", gl_thread_self ());
      gl_rwlock_wrlock (my_rwlock);
      dbgprintf ("Mutator %p after  wrlock\n", gl_thread_self ());

      i1 = random_account ();
      i2 = random_account ();
      value = ((unsigned int) rand() >> 3) % 10;
      account[i1] += value;
      account[i2] -= value;

      dbgprintf ("Mutator %p before unlock\n", gl_thread_self ());
      gl_rwlock_unlock (my_rwlock);
      dbgprintf ("Mutator %p after  unlock\n", gl_thread_self ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self ());
  return NULL;
}

static volatile int rwlock_checker_done;

static void *
rwlock_checker_thread (void *arg)
{
  while (!rwlock_checker_done)
    {
      dbgprintf ("Checker %p before check rdlock\n", gl_thread_self ());
      gl_rwlock_rdlock (my_rwlock);
      check_accounts ();
      gl_rwlock_unlock (my_rwlock);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self ());
  return NULL;
}

void
test_rwlock (void)
{
  int i;
  gl_thread_t checkerthreads[THREAD_COUNT];
  gl_thread_t threads[THREAD_COUNT];

  /* Initialization.  */
  for (i = 0; i < ACCOUNT_COUNT; i++)
    account[i] = 1000;
  rwlock_checker_done = 0;

  /* Spawn the threads.  */
  for (i = 0; i < THREAD_COUNT; i++)
    checkerthreads[i] = gl_thread_create (rwlock_checker_thread, NULL);
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] = gl_thread_create (rwlock_mutator_thread, NULL);

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i]);
  rwlock_checker_done = 1;
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (checkerthreads[i]);
  check_accounts ();
}

/* Test recursive locks by having several bank accounts and several threads
   which shuffle around money between the accounts (recursively) and another
   thread checking that all the money is still there.  */

gl_recursive_lock_define_initialized(static, my_reclock)

static void
recshuffle (void)
{
  int i1, i2, value;

  dbgprintf ("Mutator %p before lock\n", gl_thread_self ());
  gl_recursive_lock_lock (my_reclock);
  dbgprintf ("Mutator %p after  lock\n", gl_thread_self ());

  i1 = random_account ();
  i2 = random_account ();
  value = ((unsigned int) rand() >> 3) % 10;
  account[i1] += value;
  account[i2] -= value;

  /* Recursive with probability 0.5.  */
  if (((unsigned int) rand() >> 3) % 2)
    recshuffle ();

  dbgprintf ("Mutator %p before unlock\n", gl_thread_self ());
  gl_recursive_lock_unlock (my_reclock);
  dbgprintf ("Mutator %p after  unlock\n", gl_thread_self ());
}

static void *
reclock_mutator_thread (void *arg)
{
  int repeat;

  for (repeat = REPEAT_COUNT; repeat > 0; repeat--)
    {
      recshuffle ();

      dbgprintf ("Mutator %p before check lock\n", gl_thread_self ());
      gl_recursive_lock_lock (my_reclock);
      check_accounts ();
      gl_recursive_lock_unlock (my_reclock);
      dbgprintf ("Mutator %p after  check unlock\n", gl_thread_self ());

      yield ();
    }

  dbgprintf ("Mutator %p dying.\n", gl_thread_self ());
  return NULL;
}

static volatile int reclock_checker_done;

static void *
reclock_checker_thread (void *arg)
{
  while (!reclock_checker_done)
    {
      dbgprintf ("Checker %p before check lock\n", gl_thread_self ());
      gl_recursive_lock_lock (my_reclock);
      check_accounts ();
      gl_recursive_lock_unlock (my_reclock);
      dbgprintf ("Checker %p after  check unlock\n", gl_thread_self ());

      yield ();
    }

  dbgprintf ("Checker %p dying.\n", gl_thread_self ());
  return NULL;
}

void
test_recursive_lock (void)
{
  int i;
  gl_thread_t checkerthread;
  gl_thread_t threads[THREAD_COUNT];

  /* Initialization.  */
  for (i = 0; i < ACCOUNT_COUNT; i++)
    account[i] = 1000;
  reclock_checker_done = 0;

  /* Spawn the threads.  */
  checkerthread = gl_thread_create (reclock_checker_thread, NULL);
  for (i = 0; i < THREAD_COUNT; i++)
    threads[i] = gl_thread_create (reclock_mutator_thread, NULL);

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    gl_thread_join (threads[i]);
  reclock_checker_done = 1;
  gl_thread_join (checkerthread);
  check_accounts ();
}

int
main ()
{
#if TEST_PTH_THREADS
  if (!pth_init ())
    abort ();
#endif

#if DO_TEST_LOCK
  printf ("Starting test_lock ..."); fflush (stdout);
  test_lock ();
  printf (" OK\n"); fflush (stdout);
#endif
#if DO_TEST_RWLOCK
  printf ("Starting test_rwlock ..."); fflush (stdout);
  test_rwlock ();
  printf (" OK\n"); fflush (stdout);
#endif
#if DO_TEST_RECURSIVE_LOCK
  printf ("Starting test_recursive_lock ..."); fflush (stdout);
  test_recursive_lock ();
  printf (" OK\n"); fflush (stdout);
#endif

  return 0;
}

#else

/* No multithreading available.  */

int
main ()
{
  return 77;
}

#endif
