#pragma once

#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <cstdint>

static inline void pin_thread_affinity_tag(uint32_t tag) {
  if (tag == 0) return;
  thread_port_t thread = mach_thread_self();
  thread_affinity_policy_data_t policy = { (integer_t)tag };
  thread_policy_set(thread, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
  mach_port_deallocate(mach_task_self(), thread);
}

#elif defined(__linux__)
#include <pthread.h>
#include <sched.h>
#include <stdint.h>

static inline void pin_thread_to_core(int core) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

#else
static inline void pin_thread_affinity_tag(unsigned) { /* no-op */ }
static inline void pin_thread_to_core(int) { /* no-op */ }
#endif

// Convenience wrapper: on macOS use affinity tag, on Linux use core index
static inline void pin_this_thread(int id) {
#if defined(__APPLE__)
  // use a non-zero tag
  pin_thread_affinity_tag(static_cast<uint32_t>(id ? id : 1));
#elif defined(__linux__)
  pin_thread_to_core(id);
#else
  (void)id;
#endif
}
