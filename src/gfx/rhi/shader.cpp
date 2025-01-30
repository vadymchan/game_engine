
#include "gfx/rhi/shader.h"

#include "file_loader/file.h"
#include "gfx/rhi/rhi.h"

#include <filesystem>
#include <string>
#include <vector>

namespace game_engine {

// bool                 Shader::s_isRunningCheckUpdateShaderThread = false;
// std::thread          Shader::s_checkUpdateShaderThread;
// std::vector<Shader*> Shader::s_waitForUpdateShaders;
// std::map<const Shader*, std::vector<size_t>>
//     Shader::s_connectedPipelineStateHash;

Shader::~Shader() {
}

void Shader::s_startAndRunCheckUpdateShaderThread() {
  // if (!g_useRealTimeShaderUpdate) {
  //   return;
  // }
  //  Do not call this function
  assert(0);
  // static std::atomic_bool HasNewWaitForupdateShaders(false);
  // static MutexLock        lock;
  // if (!s_checkUpdateShaderThread.joinable()) {
  //   s_isRunningCheckUpdateShaderThread = true;
  //   s_checkUpdateShaderThread          = std::thread([]() {
  //     while (s_isRunningCheckUpdateShaderThread) {
  //       std::vector<Shader*> Shaders = g_rhi->getAllShaders();

  //      static int32_t CurrentIndex = 0;
  //      if (Shaders.size() > 0) {
  //        ScopedLock s(&lock);
  //        for (int32_t i = 0; i < g_maxCheckCountForRealTimeShaderUpdate;
  //             ++i, ++CurrentIndex) {
  //          if (CurrentIndex >= (int32_t)Shaders.size()) {
  //            CurrentIndex = 0;
  //          }

  //          if (Shaders[CurrentIndex]->updateShader()) {
  //            s_waitForUpdateShaders.push_back(Shaders[CurrentIndex]);
  //          }
  //        }

  //        if (s_waitForUpdateShaders.size() > 0) {
  //          HasNewWaitForupdateShaders.store(true);
  //        }
  //      }
  //      // TODO: rewrite (not portable)
  //      // Sleep(g_sleepMSForRealTimeShaderUpdate);
  //      std::this_thread::sleep_for(
  //          std::chrono::milliseconds(g_sleepMSForRealTimeShaderUpdate));
  //    }
  //  });
  //}

  // if (HasNewWaitForupdateShaders.load()) {
  //   ScopedLock s(&lock);

  //  HasNewWaitForupdateShaders.store(false);
  //  assert(s_waitForUpdateShaders.size() > 0);

  //  g_rhi->flush();

  //  for (auto it = s_waitForUpdateShaders.begin();
  //       s_waitForUpdateShaders.end() != it;) {
  //    Shader* shader = *it;

  //    // Backup previous data
  //    CompiledShader* PreviousCompiledShader = shader->m_compiledShader_;
  //    auto PreviousPipelineStateHashes = s_connectedPipelineStateHash[shader];
  //    s_connectedPipelineStateHash[shader].clear();

  //    // Reset Shader
  //    shader->m_compiledShader_ = nullptr;
  //    g_rhi->releaseShader(shader->m_shaderInfo_);

  //    // Try recreate shader
  //    if (g_rhi->createShaderInternal(shader, shader->m_shaderInfo_)) {
  //      // Remove Pipeline which is connected with this shader
  //      for (auto PipelineStateHash : PreviousPipelineStateHashes) {
  //        g_rhi->removePipelineStateInfo(PipelineStateHash);
  //      }

  //      // Release previous compiled shader
  //      delete PreviousCompiledShader;

  //      g_rhi->addShader(shader->m_shaderInfo_, shader);

  //      it = s_waitForUpdateShaders.erase(it);
  //    } else {
  //      // Restore shader data
  //      shader->m_compiledShader_            = PreviousCompiledShader;
  //      s_connectedPipelineStateHash[shader] = PreviousPipelineStateHashes;
  //      g_rhi->addShader(shader->m_shaderInfo_, shader);

  //      ++it;
  //    }
  //  }
  //}
}

void Shader::s_releaseCheckUpdateShaderThread() {
  // if (s_checkUpdateShaderThread.joinable()) {
  //   s_isRunningCheckUpdateShaderThread = false;
  //   s_checkUpdateShaderThread.join();
  // }
}

bool Shader::updateShader() {
  auto checkTimeStampFunc = [this](const char* filename) -> uint64_t {
    if (filename) {
      return File::s_getFileTimeStamp(filename);
    }
    return 0;
  };

  // Check the state of shader file or any include shader file
  uint64_t currentTimeStamp
      = checkTimeStampFunc(m_shaderInfo_.getShaderFilepath().toStr());
  for (auto Name : m_shaderInfo_.getIncludeShaderFilePaths()) {
    currentTimeStamp
        = std::max(checkTimeStampFunc(Name.toStr()), currentTimeStamp);
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

void Shader::initialize() {
  assert(g_rhi->createShaderInternal(shared_from_this(), m_shaderInfo_));
}

// GraphicsPipelineShader
size_t GraphicsPipelineShader::getHash() const {
  size_t hash = 0;

  if (m_vertexShader_) {
    hash ^= m_vertexShader_->m_shaderInfo_.getHash();
  }

  if (m_geometryShader_) {
    hash ^= m_geometryShader_->m_shaderInfo_.getHash();
  }

  if (m_pixelShader_) {
    hash ^= m_pixelShader_->m_shaderInfo_.getHash();
  }

  return hash;
}

// TODO: consider other approaches like templates (will give at least code
// highlight)
// #define IMPLEMENT_SHADER_WITH_PERMUTATION(ShaderClass, \
//                                          name, \
//                                          Filepath, \
//                                          Preprocessor, \
//                                          EntryName, \
//                                          ShaderAccesssStageFlag) \
//  ShaderInfo   ShaderClass::GShaderInfo(NameStatic(name), \
//                                      NameStatic(Filepath),                  \
//                                      NameStatic(Preprocessor),              \
//                                      NameStatic(EntryName),                 \
//                                      ShaderAccesssStageFlag);               \
//  ShaderClass* ShaderClass::CreateShader( \
//      const ShaderClass::ShaderPermutation& permutation) { \
//    ShaderInfo TempShaderInfo = GShaderInfo; \
//    TempShaderInfo.setPermutationId(permutation.getPermutationId()); \
//    ShaderClass* shader    = g_rhi->createShader<ShaderClass>(TempShaderInfo);
//    \
//    shader->m_permutation_ = permutation; \
//    return shader; \
//  }
//
// IMPLEMENT_SHADER_WITH_PERMUTATION(
//    ShaderForwardPixelShader,
//    "ForwardPS",
//    //"assets/shaders/forward_rendering/shader.ps.hlsl",
//    "assets/shaders/demo/first_triangle.ps.hlsl",  // TODO: for test
//    "",
//    "main",
//    EShaderAccessStageFlag::FRAGMENT)
//
// IMPLEMENT_SHADER_WITH_PERMUTATION(
//    ShaderGBufferVertexShader,
//    "GBufferVS",
//    "assets/shaders/deferred_rendering/gbuffer.vs.hlsl",
//    "",
//    "main",
//    EShaderAccessStageFlag::VERTEX)
//
// IMPLEMENT_SHADER_WITH_PERMUTATION(
//    ShaderGBufferPixelShader,
//    "GBufferPS",
//    "assets/shaders/deferred_rendering/gbuffer.ps.hlsl",
//    "",
//    "main",
//    EShaderAccessStageFlag::FRAGMENT)

}  // namespace game_engine
