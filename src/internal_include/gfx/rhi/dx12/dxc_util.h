#ifndef GAME_ENGINE_DXC_UTIL_DX_H
#define GAME_ENGINE_DXC_UTIL_DX_H

#include "gfx/rhi/rhi_type.h"
#include "utils/logger/global_logger.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#define DXC_COMPILER_LIBRARY L"dxcompiler.dll"
#define GET_DXC_SYMBOL       GetProcAddress
using DxcLibHandle = HMODULE;
#else
#include <dlfcn.h>
#if defined(__APPLE__)
#define DXC_COMPILER_LIBRARY "libdxcompiler.dylib"
#elif defined(__linux__)
#define DXC_COMPILER_LIBRARY "libdxcompiler.so"
#else
#error \
    "Unsupported platform. Please define DXC_COMPILER_LIBRARY for your platform."
#endif
#define GET_DXC_SYMBOL dlsym
using DxcLibHandle = void*;
#endif

#include <dxc/dxcapi.h>

namespace game_engine {

enum class ShaderBackend {
  DXIL,
  SPIRV
};

struct OptionalShaderParams {
  // include directories passed to -I
  std::vector<std::wstring> includeDirs;
  // macro definitions passed to -D
  std::vector<std::wstring> preprocessorDefs;
  // additional flags
  std::vector<std::wstring> extraArgs;
};

// helper function to wrap any DXC/COM pointer in a std::shared_ptr
template <typename T>
std::shared_ptr<T> MakeDxcSharedPtr(T* rawPtr) {
  // Custom deleter calls ->Release() when refcount hits zero
  return std::shared_ptr<T>(rawPtr, [](T* p) {
    if (p) {
      p->Release();
    }
  });
}

class DxcUtil {
  public:
  static DxcUtil& s_get() {
    static DxcUtil instance;
    if (!instance.initialize()) {
      GlobalLogger::Log(LogLevel::Error, "Failed to initialize DXC library.");
    }

    return instance;
  }

  // read from disk, then compile
  std::shared_ptr<IDxcBlob> compileHlslFile(
      const std::filesystem::path& shaderPath,
      EShaderAccessStageFlag       stage,
      const std::wstring&          entryPoint,
      ShaderBackend                backend,
      const OptionalShaderParams&  optionalParams = {}) {
    std::string code = readFile_(shaderPath);
    if (code.empty()) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to read shader file: " + shaderPath.string());
      return nullptr;
    }
    return compileHlslCode(code, stage, entryPoint, backend, optionalParams);
  }

  // compile from a raw string
  std::shared_ptr<IDxcBlob> compileHlslCode(
      const std::string&          hlslCode,
      EShaderAccessStageFlag      stage,
      const std::wstring&         entryPoint,
      ShaderBackend               backend,
      const OptionalShaderParams& optionalParams = {}) {
    if (!m_dxcCreateFn) {
      GlobalLogger::Log(LogLevel::Error, "DXC library not loaded properly.");
      return nullptr;
    }

    std::wstring targetProfile = getTargetProfile_(stage);
    if (targetProfile.empty()) {
      GlobalLogger::Log(LogLevel::Error, "Invalid shader stage provided.");
      return nullptr;
    }

    GlobalLogger::Log(
        LogLevel::Info,
        "Compiling shader for target: "
            + std::string(targetProfile.begin(), targetProfile.end()));

    IDxcCompiler3* compilerRaw = nullptr;
    IDxcUtils*     utilsRaw    = nullptr;
    if (FAILED(m_dxcCreateFn(CLSID_DxcCompiler, IID_PPV_ARGS(&compilerRaw)))
        || FAILED(m_dxcCreateFn(CLSID_DxcUtils, IID_PPV_ARGS(&utilsRaw)))) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create DXC instances.");
      return nullptr;
    }

    auto compiler = MakeDxcSharedPtr(compilerRaw);
    auto utils    = MakeDxcSharedPtr(utilsRaw);

    IDxcIncludeHandler* includeHandlerRaw = nullptr;
    if (FAILED(utils->CreateDefaultIncludeHandler(&includeHandlerRaw))) {
      return nullptr;
    }
    auto includeHandler = MakeDxcSharedPtr(includeHandlerRaw);

    DxcBuffer sourceBuf;
    sourceBuf.Ptr      = hlslCode.data();
    sourceBuf.Size     = hlslCode.size();
    sourceBuf.Encoding = DXC_CP_UTF8;

    std::vector<std::wstring> argStrings;
    std::vector<LPCWSTR>      args;

    argStrings.push_back(L"-E");
    argStrings.push_back(entryPoint);
    argStrings.push_back(L"-T");
    argStrings.push_back(targetProfile);

#ifdef _DEBUG
    argStrings.push_back(L"-Zi");
    argStrings.push_back(L"-Qembed_debug");
    argStrings.push_back(L"-Od");
#else
    argStrings.push_back(L"-O3");
#endif

    if (backend == ShaderBackend::SPIRV) {
      argStrings.push_back(L"-spirv");
    }

    // includes
    for (auto& inc : optionalParams.includeDirs) {
      argStrings.push_back(L"-I");
      argStrings.push_back(inc);
    }
    // defines
    for (auto& def : optionalParams.preprocessorDefs) {
      argStrings.push_back(L"-D");
      argStrings.push_back(def);
    }
    // extra
    for (auto& ex : optionalParams.extraArgs) {
      argStrings.push_back(ex);
    }

    // gather pointers
    args.reserve(argStrings.size());
    for (auto& s : argStrings) {
      args.push_back(s.c_str());
    }

    IDxcResult* resultRaw = nullptr;
    HRESULT     hr        = compiler->Compile(&sourceBuf,
                                   args.data(),
                                   (UINT)args.size(),
                                   includeHandler.get(),
                                   IID_PPV_ARGS(&resultRaw));
    if (FAILED(hr) || !resultRaw) {
      GlobalLogger::Log(LogLevel::Error, "Failed to compile shader.");
      return nullptr;
    }
    auto result = MakeDxcSharedPtr(resultRaw);

    IDxcBlobUtf8* errorsRaw = nullptr;
    hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorsRaw), nullptr);
    auto errorsPtr = MakeDxcSharedPtr(errorsRaw);
    if (errorsPtr && errorsPtr->GetStringLength() > 0) {
      GlobalLogger::Log(LogLevel::Warning,
                        std::string(errorsPtr->GetStringPointer(),
                                    errorsPtr->GetStringPointer()
                                        + errorsPtr->GetStringLength()));
    }

    HRESULT status;
    result->GetStatus(&status);
    if (FAILED(status)) {
      GlobalLogger::Log(LogLevel::Error, "Shader compilation failed.");
      return nullptr;
    }

    IDxcBlob* blobRaw = nullptr;
    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&blobRaw), nullptr);
    if (FAILED(hr) || !blobRaw) {
      GlobalLogger::Log(LogLevel::Error, "Failed to retrieve shader blob.");
      return nullptr;
    }

    return MakeDxcSharedPtr(blobRaw);
  }

  private:
  bool initialize() {
    if (m_libHandle) {
      return true;
    }

#if defined(_WIN32)
    m_libHandle = LoadLibraryW(DXC_COMPILER_LIBRARY);
#else
    m_libHandle = dlopen(DXC_COMPILER_LIBRARY, RTLD_LAZY);
#endif

    if (!m_libHandle) {
      return false;
    }

    m_dxcCreateFn = reinterpret_cast<DxcCreateInstanceProc>(
        GET_DXC_SYMBOL(m_libHandle, "DxcCreateInstance"));

    if (!m_dxcCreateFn) {
#if defined(_WIN32)
      FreeLibrary(m_libHandle);
#else
      dlclose(m_libHandle);
#endif
      m_libHandle = nullptr;
      return false;
    }

    return true;
  }

  std::wstring getTargetProfile_(EShaderAccessStageFlag stage) {
    static const std::wstring suffix = L"_6_6";
    switch (stage) {
      case EShaderAccessStageFlag::VERTEX:
        return L"vs" + suffix;
      case EShaderAccessStageFlag::FRAGMENT:
        return L"ps" + suffix;
      case EShaderAccessStageFlag::COMPUTE:
        return L"cs" + suffix;
      case EShaderAccessStageFlag::GEOMETRY:
        return L"gs" + suffix;
      case EShaderAccessStageFlag::TESSELLATION_CONTROL:
        return L"hs" + suffix;
      case EShaderAccessStageFlag::TESSELLATION_EVALUATION:
        return L"ds" + suffix;

      case EShaderAccessStageFlag::RAYTRACING:
      case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
      case EShaderAccessStageFlag::RAYTRACING_MISS:
      case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
      case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
        return L"lib" + suffix;
      default:
        return L"";  // unsupported combination
    }
  }

  static std::string readFile_(const std::filesystem::path& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to open shader file: " + path.string());
      return {};
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
  }

  private:
  DxcUtil() = default;

  ~DxcUtil() {
    if (m_libHandle) {
#if defined(_WIN32)
      FreeLibrary(m_libHandle);
#else
      dlclose(m_libHandle);
#endif
      m_libHandle = nullptr;
    }
  }

  DxcUtil(const DxcUtil&)            = delete;
  DxcUtil& operator=(const DxcUtil&) = delete;

  private:
  DxcLibHandle          m_libHandle   = nullptr;
  DxcCreateInstanceProc m_dxcCreateFn = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DXC_UTIL_DX_H
