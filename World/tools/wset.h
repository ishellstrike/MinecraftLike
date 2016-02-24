#pragma once
#ifndef WSET_H__
#define WSET_H__
#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#ifdef __APPLE__
#include <mach/task.h>
#include <mach/mach_init.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <Psapi.h>
#else
#include <sys/resource.h>
#endif

/// The amount of memory currently being used by this process, in bytes.
/// By default, returns the full virtual arena, but if resident=true,
/// it will report just the resident set in RAM (if supported on that OS).
inline size_t memory_used(bool resident = false)
{
#if defined(__linux__)
  // Ugh, getrusage doesn't work well on Linux.  Try grabbing info
  // directly from the /proc pseudo-filesystem.  Reading from
  // /proc/self/statm gives info on your own process, as one line of
  // numbers that are: virtual mem program size, resident set size,
  // shared pages, text/code, data/stack, library, dirty pages.  The
  // mem sizes should all be multiplied by the page size.
  size_t size = 0;
  FILE *file = fopen("/proc/self/statm", "r");
  if (file) {
    unsigned long vm = 0;
    fscanf(file, "%ul", &vm);  // Just need the first num: vm size
    fclose(file);
    size = (size_t)vm * getpagesize();
  }
  return size;

#elif defined(__APPLE__)
  // Inspired by:
  // http://miknight.blogspot.com/2005/11/resident-set-size-in-mac-os-x.html
  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
  task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
  size_t size = (resident ? t_info.resident_size : t_info.virtual_size);
  return size;

#elif defined(_WIN32)
  // According to MSDN...
  PROCESS_MEMORY_COUNTERS counters;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
    return counters.PagefileUsage;
  else return 0;

#else
  // No idea what platform this is
  return 0;   // Punt
#endif
}
#endif //WSET_H__