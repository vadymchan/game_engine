#ifndef GAME_ENGINE_TRACY_CONFIG_H
#define GAME_ENGINE_TRACY_CONFIG_H

#ifdef GAME_ENGINE_USE_TRACY
#define TRACY_ENABLE
#include <tracy/Tracy.hpp>

#define PROFILE_FRAME()                                         FrameMark
#define PROFILE_SCOPE(name)                                     ZoneScoped
#define PROFILE_SCOPE_N(name)                                   ZoneScopedN(name)
#define PROFILE_GPU_ZONE(cmd, name)                             // Platform-specific, TODO

#define PROFILE_ALLOC(ptr, size)                                TracyAlloc(ptr, size)
#define PROFILE_FREE(ptr)                                       TracyFree(ptr)

#define PROFILE_PLOT(name, value)                               TracyPlot(name, value)

#define PROFILE_SCOPE_C(name, color)                            ZoneScopedC(color)
#define PROFILE_SCOPE_NC(name, color)                           ZoneScopedNC(name, color)

#define PROFILE_MESSAGE(text, size)                             TracyMessage(text, size)
#define PROFILE_MESSAGE_L(text)                                 TracyMessageL(text)
#define PROFILE_VALUE(text, value)                              TracyMessageL(text ": " + std::to_string(value))

#define PROFILE_SET_THREAD_NAME(name)                           TracySetThreadName(name)

#define PROFILE_LOCKABLE(type, var)                             TracyLockable(type, var)
#define PROFILE_LOCK_MARK(var)                                  LockMark(var)

#define PROFILE_APP_INFO(name, size)                            TracyAppInfo(name, size)

#define PROFILE_FRAME_IMAGE(image, width, height, offset, flip) TracyFrameImage(image, width, height, offset, flip)

#else
#define PROFILE_FRAME()
#define PROFILE_SCOPE(name)
#define PROFILE_SCOPE_N(name)
#define PROFILE_GPU_ZONE(cmd, name)
#define PROFILE_ALLOC(ptr, size)
#define PROFILE_FREE(ptr)
#define PROFILE_PLOT(name, value)
#define PROFILE_SCOPE_C(name, color)
#define PROFILE_SCOPE_NC(name, color)
#define PROFILE_MESSAGE(text, size)
#define PROFILE_MESSAGE_L(text)
#define PROFILE_VALUE(text, value)
#define PROFILE_SET_THREAD_NAME(name)
#define PROFILE_LOCKABLE(type, var)
#define PROFILE_LOCK_MARK(var)
#define PROFILE_APP_INFO(name, size)
#define PROFILE_FRAME_IMAGE(image, width, height, offset, flip)
#endif

#endif  // GAME_ENGINE_TRACY_CONFIG_H