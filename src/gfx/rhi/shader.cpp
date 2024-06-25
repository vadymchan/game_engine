
#include "gfx/rhi/shader.h"

#include "file_loader/file.h"
#include "gfx/rhi/rhi.h"

#include <filesystem>
#include <string>
#include <vector>

namespace game_engine {

bool                 Shader::IsRunningCheckUpdateShaderThread = false;
std::thread          Shader::CheckUpdateShaderThread;
std::vector<Shader*> Shader::WaitForUpdateShaders;
std::map<const Shader*, std::vector<size_t>>
    Shader::gConnectedPipelineStateHash;

Shader::~Shader() {
  delete CompiledShader;
}

void Shader::StartAndRunCheckUpdateShaderThread() {
  if (!GUseRealTimeShaderUpdate) {
    return;
  }

  static std::atomic_bool HasNewWaitForupdateShaders(false);
  static MutexLock       Lock;
  if (!CheckUpdateShaderThread.joinable()) {
    IsRunningCheckUpdateShaderThread = true;
    CheckUpdateShaderThread          = std::thread([]() {
      while (IsRunningCheckUpdateShaderThread) {
        std::vector<Shader*> Shaders = g_rhi->GetAllShaders();

        static int32_t CurrentIndex = 0;
        if (Shaders.size() > 0) {
          ScopedLock s(&Lock);
          for (int32_t i = 0; i < GMaxCheckCountForRealTimeShaderUpdate;
               ++i, ++CurrentIndex) {
            if (CurrentIndex >= (int32_t)Shaders.size()) {
              CurrentIndex = 0;
            }

            if (Shaders[CurrentIndex]->UpdateShader()) {
              WaitForUpdateShaders.push_back(Shaders[CurrentIndex]);
            }
          }

          if (WaitForUpdateShaders.size() > 0) {
            HasNewWaitForupdateShaders.store(true);
          }
        }
        // TODO: rewrite (not portable)
        // Sleep(GSleepMSForRealTimeShaderUpdate);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(GSleepMSForRealTimeShaderUpdate));
      }
    });
  }

  if (HasNewWaitForupdateShaders.load()) {
    ScopedLock s(&Lock);

    HasNewWaitForupdateShaders.store(false);
    assert(WaitForUpdateShaders.size() > 0);

    g_rhi->Flush();

    for (auto it = WaitForUpdateShaders.begin();
         WaitForUpdateShaders.end() != it;) {
      Shader* shader = *it;

      // Backup previous data
      jCompiledShader* PreviousCompiledShader = shader->CompiledShader;
      auto PreviousPipelineStateHashes = gConnectedPipelineStateHash[shader];
      gConnectedPipelineStateHash[shader].clear();

      // Reset Shader
      shader->CompiledShader = nullptr;
      g_rhi->ReleaseShader(shader->shaderInfo);

      // Try recreate shader
      if (g_rhi->CreateShaderInternal(shader, shader->shaderInfo)) {
        // Remove Pipeline which is connected with this shader
        for (auto PipelineStateHash : PreviousPipelineStateHashes) {
          g_rhi->RemovePipelineStateInfo(PipelineStateHash);
        }

        // Release previous compiled shader
        delete PreviousCompiledShader;

        g_rhi->AddShader(shader->shaderInfo, shader);

        it = WaitForUpdateShaders.erase(it);
      } else {
        // Restore shader data
        shader->CompiledShader              = PreviousCompiledShader;
        gConnectedPipelineStateHash[shader] = PreviousPipelineStateHashes;
        g_rhi->AddShader(shader->shaderInfo, shader);

        ++it;
      }
    }
  }
}

void Shader::ReleaseCheckUpdateShaderThread() {
  if (CheckUpdateShaderThread.joinable()) {
    IsRunningCheckUpdateShaderThread = false;
    CheckUpdateShaderThread.join();
  }
}

bool Shader::UpdateShader() {
  auto checkTimeStampFunc = [this](const char* filename) -> uint64_t {
    if (filename) {
      return File::GetFileTimeStamp(filename);
    }
    return 0;
  };

  // Check the state of shader file or any include shader file
  uint64_t currentTimeStamp
      = checkTimeStampFunc(shaderInfo.GetShaderFilepath().ToStr());
  for (auto Name : shaderInfo.GetIncludeShaderFilePaths()) {
    currentTimeStamp
        = std::max(checkTimeStampFunc(Name.ToStr()), currentTimeStamp);
  }

  if (currentTimeStamp <= 0) {
    return false;
  }

  if (TimeStamp == 0) {
    TimeStamp = currentTimeStamp;
    return false;
  }

  if (TimeStamp >= currentTimeStamp) {
    return false;
  }

  TimeStamp = currentTimeStamp;

  return true;
}

void Shader::Initialize() {
  assert(g_rhi->CreateShaderInternal(this, shaderInfo));
}

// GraphicsPipelineShader
size_t GraphicsPipelineShader::GetHash() const {
  size_t hash = 0;

  if (VertexShader) {
    hash ^= VertexShader->shaderInfo.GetHash();
  }

  if (GeometryShader) {
    hash ^= GeometryShader->shaderInfo.GetHash();
  }

  if (PixelShader) {
    hash ^= PixelShader->shaderInfo.GetHash();
  }

  return hash;
}

#define IMPLEMENT_SHADER_WITH_PERMUTATION(ShaderClass,                      \
                                          name,                             \
                                          Filepath,                         \
                                          Preprocessor,                     \
                                          EntryName,                        \
                                          ShaderAccesssStageFlag)           \
  ShaderInfo   ShaderClass::GShaderInfo(NameStatic(name),                   \
                                      NameStatic(Filepath),               \
                                      NameStatic(Preprocessor),           \
                                      NameStatic(EntryName),              \
                                      ShaderAccesssStageFlag);            \
  ShaderClass* ShaderClass::CreateShader(                                   \
      const ShaderClass::ShaderPermutation& InPermutation) {                \
    ShaderInfo TempShaderInfo = GShaderInfo;                                \
    TempShaderInfo.SetPermutationId(InPermutation.GetPermutationId());      \
    ShaderClass* shader = g_rhi->CreateShader<ShaderClass>(TempShaderInfo); \
    shader->permutation = InPermutation;                                    \
    return shader;                                                          \
  }

IMPLEMENT_SHADER_WITH_PERMUTATION(
    ShaderForwardPixelShader,
    "ForwardPS",
    //"assets/shaders/forward_rendering/shader.ps.hlsl",
    "assets/shaders/demo/first_triangle.ps.hlsl",  // TODO: for test
    "",
    "main",
    EShaderAccessStageFlag::FRAGMENT)

IMPLEMENT_SHADER_WITH_PERMUTATION(
    ShaderGBufferVertexShader,
    "GBufferVS",
    "assets/shaders/deferred_rendering/gbuffer.vs.hlsl",
    "",
    "main",
    EShaderAccessStageFlag::VERTEX)

IMPLEMENT_SHADER_WITH_PERMUTATION(
    ShaderGBufferPixelShader,
    "GBufferPS",
    "assets/shaders/deferred_rendering/gbuffer.ps.hlsl",
    "",
    "main",
    EShaderAccessStageFlag::FRAGMENT)

}  // namespace game_engine
