#ifndef GAME_ENGINE_DXC_UTIL_DX_H
#define GAME_ENGINE_DXC_UTIL_DX_H

// TODO: all this class is need to be refactored completely!!!

#include "gfx/rhi/rhi_type.h"
#include "platform/windows/windows_platform_setup.h"
#include "utils/logger/global_logger.h"

#include <windows.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace game_engine {

namespace fs = std::filesystem;

class DxcUtil {
  public:
  // TODO: consider whether this is the good naming convention
  static DxcUtil& s_getInstance(const wchar_t* dllName = L"dxcompiler.dll",
                                const char*    fnName  = "DxcCreateInstance") {
    static DxcUtil instance;
    // TODO: not good solution
    instance.initialize(dllName, fnName);
    return instance;
  }

  HRESULT initialize(const wchar_t* dllName = L"dxcompiler.dll",
                     const char*    fnName  = "DxcCreateInstance") {
    if (m_dll_) {
      return S_OK;
    }

    m_dll_ = LoadLibraryW(dllName);
    if (!m_dll_) {
      return HRESULT_FROM_WIN32(GetLastError());
    }

    m_createFn_ = (DxcCreateInstanceProc)GetProcAddress(m_dll_, fnName);
    if (!m_createFn_) {
      HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
      FreeLibrary(m_dll_);
      m_dll_ = nullptr;
      return hr;
    }

    return S_OK;
  }

  void release() {
    if (m_dll_) {
      m_createFn_ = nullptr;
      FreeLibrary(m_dll_);
      m_dll_ = nullptr;
    }
  }

  ComPtr<IDxcBlob> compileHlslFileToDxil(const fs::path&     shaderFilePath,
                                         const std::wstring& shaderProfile,
                                         const std::wstring& entryPoint) {
    std::string hlslCode = s_readShaderFile_(shaderFilePath);
    return compileHlslCodeToDxil(hlslCode, shaderProfile, entryPoint);
  }

  ComPtr<IDxcBlob> compileHlslCodeToDxil(const std::string&  hlslCode,
                                         const std::wstring& shaderProfile,
                                         const std::wstring& entryPoint) {
    ComPtr<IDxcCompiler3>      compiler;
    ComPtr<IDxcUtils>          utils;
    ComPtr<IDxcIncludeHandler> includeHandler;

    if (FAILED(createInstance_(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)))
        || FAILED(createInstance_(CLSID_DxcUtils, IID_PPV_ARGS(&utils)))
        || FAILED(utils->CreateDefaultIncludeHandler(&includeHandler))) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create DXC instances");
      return nullptr;
    }

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr      = hlslCode.data();
    sourceBuffer.Size     = hlslCode.size();
    sourceBuffer.Encoding = DXC_CP_ACP;

    std::vector<LPCWSTR> arguments = {
      L"-E",
      entryPoint.c_str(),           // Entry point.
      L"-T",
      shaderProfile.c_str(),        // Shader model.
      DXC_ARG_WARNINGS_ARE_ERRORS,  // Warnings as errors.
      DXC_ARG_DEBUG,                // Enable debug information.
#ifdef _DEBUG
      L"-Zi",                       // Debug info.
      L"-Qembed_debug",             // Embed PDB in shader container.
      L"-Od"                        // Disable optimization.
#else
      L"-O3"  // Optimization Level 3 (Default).
#endif
    };

    ComPtr<IDxcResult> result;
    if (FAILED(compiler->Compile(&sourceBuffer,
                                 arguments.data(),
                                 arguments.size(),
                                 includeHandler.Get(),
                                 IID_PPV_ARGS(&result)))) {
      GlobalLogger::Log(LogLevel::Error, "Failed to compile shader");
      return nullptr;
    }

    ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

    if (errors != nullptr && errors->GetStringLength() > 0) {
      GlobalLogger::Log(LogLevel::Warning, errors->GetStringPointer());
    }

    HRESULT resultCode;
    if (FAILED(result->GetStatus(&resultCode))) {
      GlobalLogger::Log(LogLevel::Error,
                        "Compilation failed with status error");
      return nullptr;
    }

    ComPtr<IDxcBlob> shader;
    if (FAILED(result->GetOutput(
            DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr))) {
      GlobalLogger::Log(LogLevel::Error, "Failed to get shader blob");
      return nullptr;
    }

    return shader;
  }

  private:
  DxcUtil() = default;

  ~DxcUtil() { release(); }

  DxcUtil(const DxcUtil&)            = delete;
  DxcUtil& operator=(const DxcUtil&) = delete;

  HRESULT createInstance_(REFCLSID clsid, REFIID iid, void** result) {
    if (!result) {
      return E_POINTER;
    }
    if (!m_dll_) {
      return E_FAIL;
    }
    if (!m_createFn_) {
      return E_FAIL;
    }
    return m_createFn_(clsid, iid, result);
  }

  static std::string s_readShaderFile_(const fs::path& shaderFilePath) {
    std::ifstream shaderFile(shaderFilePath);
    if (!shaderFile) {
      GlobalLogger::Log(
          LogLevel::Error,
          "Failed to open shader file: " + shaderFilePath.string());
      return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
  }

  HMODULE               m_dll_      = nullptr;
  DxcCreateInstanceProc m_createFn_ = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DXC_UTIL_DX_H
