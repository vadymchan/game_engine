#ifndef GAME_ENGINE_SPIRV_UTIL_VK_H
#define GAME_ENGINE_SPIRV_UTIL_VK_H

#include "gfx/rhi/rhi_type.h"
#include "utils/logger/global_logger.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <shaderc/shaderc.hpp>

namespace game_engine {

namespace fs = std::filesystem;

class SpirvUtil {
  public:
  static std::vector<uint32_t> compileHlslFileToSpirv(
      const fs::path&     shaderFilePath,
      shaderc_shader_kind shaderKind,
      const std::string&  entryPoint) {
    // Read HLSL code from file
    std::string hlslCode = ReadShaderFile(shaderFilePath);

    static shaderc::Compiler       compiler;
    static shaderc::CompileOptions options = []() {
      shaderc::CompileOptions opts;
      opts.SetTargetEnvironment(s_kTargetEnvironment, s_kTargetVersion);
      opts.SetSourceLanguage(s_kSourceLanguage);
      return opts;
    }();

    shaderc::SpvCompilationResult compilationResult
        = compiler.CompileGlslToSpv(hlslCode,
                                    shaderKind,
                                    shaderFilePath.string().c_str(),
                                    entryPoint.c_str(),
                                    options);

    if (compilationResult.GetCompilationStatus()
        != shaderc_compilation_status_success) {
      GlobalLogger::Log(LogLevel::Error, compilationResult.GetErrorMessage());
      return {};
    }

    return {compilationResult.cbegin(), compilationResult.cend()};
  }

  static std::vector<uint32_t> compileHlslCodeToSpirv(
      const std::string&  hlslCode,
      shaderc_shader_kind shaderKind,
      const std::string&  entryPoint) {
    static shaderc::Compiler       compiler;
    static shaderc::CompileOptions options = []() {
      shaderc::CompileOptions opts;
      opts.SetTargetEnvironment(s_kTargetEnvironment, s_kTargetVersion);
      opts.SetSourceLanguage(s_kSourceLanguage);
      opts.SetGenerateDebugInfo();
      return opts;
    }();

    shaderc::SpvCompilationResult compilationResult = compiler.CompileGlslToSpv(
        hlslCode, shaderKind, "shader.hlsl", entryPoint.c_str(), options);

    if (compilationResult.GetCompilationStatus()
        != shaderc_compilation_status_success) {
      GlobalLogger::Log(LogLevel::Error, compilationResult.GetErrorMessage());
      return {};
    }

    return {compilationResult.cbegin(), compilationResult.cend()};
  }

  static shaderc_shader_kind getShadercShaderKind(EShaderAccessStageFlag flag) {
    switch (flag) {
      case EShaderAccessStageFlag::VERTEX:
        return shaderc_glsl_vertex_shader;
      case EShaderAccessStageFlag::FRAGMENT:
        return shaderc_glsl_fragment_shader;
      case EShaderAccessStageFlag::COMPUTE:
        return shaderc_glsl_compute_shader;
      case EShaderAccessStageFlag::GEOMETRY:
        return shaderc_glsl_geometry_shader;
      case EShaderAccessStageFlag::TESSELLATION_CONTROL:
        return shaderc_glsl_tess_control_shader;
      case EShaderAccessStageFlag::TESSELLATION_EVALUATION:
        return shaderc_glsl_tess_evaluation_shader;
      case EShaderAccessStageFlag::RAYTRACING_RAYGEN:
        return shaderc_raygen_shader;
      case EShaderAccessStageFlag::RAYTRACING_MISS:
        return shaderc_miss_shader;
      case EShaderAccessStageFlag::RAYTRACING_CLOSESTHIT:
        return shaderc_closesthit_shader;
      case EShaderAccessStageFlag::RAYTRACING_ANYHIT:
        return shaderc_anyhit_shader;
      default:
        GlobalLogger::Log(LogLevel::Error, "Unsupported shader stage flag");
        return shaderc_glsl_infer_from_source;
    }
  }

  private:
  static std::string ReadShaderFile(const fs::path& shaderFilePath) {
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

  // TODO: make it configurable
  static constexpr auto s_kTargetEnvironment = shaderc_target_env_vulkan;
  // TODO: select depending on selected vulkan version in vkInstance
  static constexpr auto s_kTargetVersion  = shaderc_env_version_vulkan_1_3;
  static constexpr auto s_kSourceLanguage = shaderc_source_language_hlsl;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_SPIRV_UTIL_VK_H
