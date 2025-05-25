#ifndef GAME_ENGINE_PROFILER_CPU_H
#define GAME_ENGINE_PROFILER_CPU_H

#ifdef GAME_ENGINE_USE_CPU_PROFILING

#include "utils/color/color.h"

#ifdef GAME_ENGINE_USE_TRACY
//#define TRACY_ENABLE
#include <tracy/Tracy.hpp>

#define CPU_ZONE()                         ZoneScoped
#define CPU_ZONE_N(name)                   ZoneScopedN(name)

// for color you can use predefined colors or g_toFloatArray function from color namespace
#define CPU_ZONE_C(color)                  ZoneScopedC(color)
#define CPU_ZONE_NC(name, color)           ZoneScopedNC(name, color)

// Lockable objects tracking
#define CPU_LOCKABLE(type, varname)        TracyLockable(type, varname)
#define CPU_SHARED_LOCKABLE(type, varname) TracySharedLockable(type, varname)

// Lock marking
#define CPU_LOCK_MARK(varname)             LockMark(varname)

// Automatic function profiling
#define CPU_FUNCTION() \
  ZoneScoped;          \
  ZoneName(__FUNCTION__, strlen(__FUNCTION__))
#define CPU_METHOD() \
  ZoneScoped;        \
  ZoneName(__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__))


#else
#define CPU_ZONE()
#define CPU_ZONE_N(name)
#define CPU_ZONE_C(color)
#define CPU_ZONE_NC(name, color)
#define CPU_LOCKABLE(type, varname)
#define CPU_SHARED_LOCKABLE(type, varname)
#define CPU_LOCK_MARK(varname)
#define CPU_FUNCTION()
#define CPU_METHOD()

#endif

#endif  // GAME_ENGINE_USE_CPU_PROFILING

#endif
