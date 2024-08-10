
#ifndef GAME_ENGINE_WINDOWS_PLATFORM_SETUP_H
#define GAME_ENGINE_WINDOWS_PLATFORM_SETUP_H

// TODO: think about what include to use and what version (e.g. dxgi1_6)
#include <directx/d3dx12.h>
#include <d3d12.h>
#include <dxcapi.h>   // for shader compilation
#include <dxgi1_6.h>
#include <windows.h>  // Win API
#include <wrl.h>      // manages the lifetime of COM objects and interfaces

using Microsoft::WRL::ComPtr;

#ifdef max
  #undef max
#endif

#ifdef min
  #undef min
#endif

#endif  // GAME_ENGINE_WINDOWS_PLATFORM_SETUP_H
