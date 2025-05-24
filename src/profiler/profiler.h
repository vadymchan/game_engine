#ifndef GAME_ENGINE_PROFILER_H
#define GAME_ENGINE_PROFILER_H

// #ifdef GAME_ENGINE_USE_PROFILING

#include "profiler/gpu_profiler.h"

#ifdef GAME_ENGINE_USE_TRACY
#define TRACY_ENABLE
#include <tracy/Tracy.hpp>

#define PROFILER_FRAME()                                         FrameMark
#define PROFILER_SCOPE(name)                                     ZoneScoped
#define PROFILER_SCOPE_N(name)                                   ZoneScopedN(name)
// TODO: remove (implement right in rhi side - command buffer / device / swapchain)
#define PROFILER_GPU_ZONE(cmd, name)                             game_engine::profiler::gpu::Marker _gpuMarker##__LINE__(cmd, name)

#define PROFILER_ALLOC(ptr, size)                                TracyAlloc(ptr, size)
#define PROFILER_FREE(ptr)                                       TracyFree(ptr)

#define PROFILER_PLOT(name, value)                               TracyPlot(name, value)

#define PROFILER_SCOPE_C(name, color)                            ZoneScopedC(color)
#define PROFILER_SCOPE_NC(name, color)                           ZoneScopedNC(name, color)

#define PROFILER_MESSAGE(text, size)                             TracyMessage(text, size)
#define PROFILER_MESSAGE_L(text)                                 TracyMessageL(text)
#define PROFILER_VALUE(text, value)                              TracyMessageL(text ": " + std::to_string(value))

#define PROFILER_LOCKABLE(type, var)                             TracyLockable(type, var)
#define PROFILER_LOCK_MARK(var)                                  LockMark(var)

#define PROFILER_APP_INFO(name, size)                            TracyAppInfo(name, size)

#define PROFILER_FRAME_IMAGE(image, width, height, offset, flip) FrameImage(image, width, height, offset, flip)

#else

#define PROFILER_FRAME()
#define PROFILER_SCOPE(name)
#define PROFILER_SCOPE_N(name)
#define PROFILER_GPU_ZONE(cmd, name)
#define PROFILER_ALLOC(ptr, size)
#define PROFILER_FREE(ptr)
#define PROFILER_PLOT(name, value)
#define PROFILER_SCOPE_C(name, color)
#define PROFILER_SCOPE_NC(name, color)
#define PROFILER_MESSAGE(text, size)
#define PROFILER_MESSAGE_L(text)
#define PROFILER_VALUE(text, value)
#define PROFILER_LOCKABLE(type, var)
#define PROFILER_LOCK_MARK(var)
#define PROFILER_APP_INFO(name, size)
#define PROFILER_FRAME_IMAGE(image, width, height, offset, flip)
#endif

// #endif  // GAME_ENGINE_PROFILER_H

#endif
