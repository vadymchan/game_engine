
#include "gfx/rhi/shader.h"

#include "file_loader/file.h"
#include "gfx/rhi/rhi.h"

#include <filesystem>
#include <string>
#include <vector>

namespace game_engine {

bool                 Shader::s_isRunningCheckUpdateShaderThread = false;
std::thread          Shader::s_checkUpdateShaderThread;
std::vector<Shader*> Shader::s_waitForUpdateShaders;
std::map<const Shader*, std::vector<size_t>>
    Shader::s_connectedPipelineStateHash;

Shader::~Shader() {
  delete m_compiledShader;
}

void Shader::StartAndRunCheckUpdateShaderThread() {
  if (!g_useRealTimeShaderUpdate) {
    return;
  }

  static std::atomic_bool HasNewWaitForupdateShaders(false);
  static MutexLock        Lock;
  if (!s_checkUpdateShaderThread.joinable()) {
    s_isRunningCheckUpdateShaderThread = true;
    s_checkUpdateShaderThread          = std::thread([]() {
      while (s_isRunningCheckUpdateShaderThread) {
        std::vector<Shader*> Shaders = g_rhi->GetAllShaders();

        static int32_t CurrentIndex = 0;
        if (Shaders.size() > 0) {
          ScopedLock s(&Lock);
          for (int32_t i = 0; i < g_maxCheckCountForRealTimeShaderUpdate;
               ++i, ++CurrentIndex) {
            if (CurrentIndex >= (int32_t)Shaders.size()) {
              CurrentIndex = 0;
            }

            if (Shaders[CurrentIndex]->UpdateShader()) {
              s_waitForUpdateShaders.push_back(Shaders[CurrentIndex]);
            }
          }

          if (s_waitForUpdateShaders.size() > 0) {
            HasNewWaitForupdateShaders.store(true);
          }
        }
        // TODO: rewrite (not portable)
        // Sleep(g_sleepMSForRealTimeShaderUpdate);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(g_sleepMSForRealTimeShaderUpdate));
      }
    });
  }

  if (HasNewWaitForupdateShaders.load()) {
    ScopedLock s(&Lock);

    HasNewWaitForupdateShaders.store(false);
    assert(s_waitForUpdateShaders.size() > 0);

    g_rhi->Flush();

    for (auto it = s_waitForUpdateShaders.begin();
         s_waitForUpdateShaders.end() != it;) {
      Shader* shader = *it;

      // Backup previous data
      CompiledShader* PreviousCompiledShader = shader->m_compiledShader;
      auto PreviousPipelineStateHashes = s_connectedPipelineStateHash[shader];
      s_connectedPipelineStateHash[shader].clear();

      // Reset Shader
      shader->m_compiledShader = nullptr;
      g_rhi->ReleaseShader(shader->m_shaderInfo_);

      // Try recreate shader
      if (g_rhi->CreateShaderInternal(shader, shader->m_shaderInfo_)) {
        // Remove Pipeline which is connected with this shader
        for (auto PipelineStateHash : PreviousPipelineStateHashes) {
          g_rhi->RemovePipelineStateInfo(PipelineStateHash);
        }

        // Release previous compiled shader
        delete PreviousCompiledShader;

        g_rhi->AddShader(shader->m_shaderInfo_, shader);

        it = s_waitForUpdateShaders.erase(it);
      } else {
        // Restore shader data
        shader->m_compiledShader             = PreviousCompiledShader;
        s_connectedPipelineStateHash[shader] = PreviousPipelineStateHashes;
        g_rhi->AddShader(shader->m_shaderInfo_, shader);

        ++it;
      }
    }
  }
}

void Shader::ReleaseCheckUpdateShaderThread() {
  if (s_checkUpdateShaderThread.joinable()) {
    s_isRunningCheckUpdateShaderThread = false;
    s_checkUpdateShaderThread.join();
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
      = checkTimeStampFunc(m_shaderInfo_.GetShaderFilepath().ToStr());
  for (auto Name : m_shaderInfo_.GetIncludeShaderFilePaths()) {
    currentTimeStamp
        = std::max(checkTimeStampFunc(Name.ToStr()), currentTimeStamp);
  }

  if (currentTimeStamp <= 0) {
    return false;
  }

  if (m_timeStamp_ == 0) {
    m_timeStamp_ = currentTimeStamp;
    return false;
  }

  if (m_timeStamp_ >= currentTimeStamp) {
    return false;
  }

  m_timeStamp_ = currentTimeStamp;

  return true;
}

void Shader::Initialize() {
  assert(g_rhi->CreateShaderInternal(this, m_shaderInfo_));
}

// GraphicsPipelineShader
size_t GraphicsPipelineShader::GetHash() const {
  size_t hash = 0;

  if (m_vertexShader_) {
    hash ^= m_vertexShader_->m_shaderInfo_.GetHash();
  }

  if (m_geometryShader_) {
    hash ^= m_geometryShader_->m_shaderInfo_.GetHash();
  }

  if (m_pixelShader_) {
    hash ^= m_pixelShader_->m_shaderInfo_.GetHash();
  }

  return hash;
}

#define IMPLEMENT_SHADER_WITH_PERMUTATION(ShaderClass,                         \
                                          name,                                \
                                          Filepath,                            \
                                          Preprocessor,                        \
                                          EntryName,                           \
                                          ShaderAccesssStageFlag)              \
  ShaderInfo   ShaderClass::GShaderInfo(NameStatic(name),                      \
                                      NameStatic(Filepath),                  \
                                      NameStatic(Preprocessor),              \
                                      NameStatic(EntryName),                 \
                                      ShaderAccesssStageFlag);               \
  ShaderClass* ShaderClass::CreateShader(                                      \
      const ShaderClass::ShaderPermutation& InPermutation) {                   \
    ShaderInfo TempShaderInfo = GShaderInfo;                                   \
    TempShaderInfo.SetPermutationId(InPermutation.GetPermutationId());         \
    ShaderClass* shader    = g_rhi->CreateShader<ShaderClass>(TempShaderInfo); \
    shader->m_permutation_ = InPermutation;                                    \
    return shader;                                                             \
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
