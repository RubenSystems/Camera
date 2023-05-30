#include <stdexcept>
#include <threading.h>

void set_thread_affinity(std::thread &thread, int core) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  int rc = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t),
                                  &cpuset);
  if (rc < 0) {
    throw std::runtime_error("[THREADING] - setting thread affinity");
  }
}