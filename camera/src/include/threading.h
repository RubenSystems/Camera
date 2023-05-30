#pragma once

#include <pthread.h>
#include <sched.h>
#include <thread>

void set_thread_affinity(std::thread &thread, int core);