#ifndef GAME_ENGINE_SHADER_H
#define GAME_ENGINE_SHADER_H

#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"  // TODO: check and remove if not needed
#include "gfx/rhi/vulkan/spirv_util.h"

#include <vulkan/vulkan.h>

#include <filesystem>
#include <iterator>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace game_engine {

// Shader Permutation Define
#define DECLARE_DEFINE(Name, ...)                          \
  class Name {                                             \
    public:                                                \
    static constexpr char DefineName[] = #Name;            \
    static constexpr int  Value[]      = {__VA_ARGS__};    \
    static constexpr int  Count        = std::size(Value); \
  };

// Shader Permutation. by using Shader Permutation Define, generate m_permutation_
// of defines and convert it to m_permutation_ id.
template <typename... T>
class Permutation {
  public:
  static int GetPermutationCount() { return 1; }

  template <typename K>
  static int GetDefineCount() {
    return 1;
  }

  template <typename K>
  int Get() const {
    return int();
  }

  template <typename K>
  int GetIndex() const {
    return int();
  }

  template <typename K>
  void SetIndex(int) {}

  int GetPermutationId() const { return 0; }

  void SetFromPermutationId(int) {}

  void GetPermutationDefines(std::string&) const {}
};

template <typename T, typename... T1>
class Permutation<T, T1...> : public Permutation<T1...> {
  public:
  using Type  = T;
  using Super = Permutation<T1...>;

  static int GetPermutationCount() {
    return T::Count * Super::GetPermutationCount();
  }

  template <typename K>
  static int GetDefineCount() {
    if (std::is_same<T, K>::value) {
      return T::Count;
    }

    return Super::template GetDefineCount<K>();
  }

  template <typename K>
  int Get() const {
    if (std::is_same<T, K>::value) {
      return T::Value[ValueIndex];
    }

    return Super::template Get<K>();
  }

  template <typename K>
  int GetIndex() const {
    if (std::is_same<T, K>::value) {
      return ValueIndex;
    }

    return Super::template GetIndex<K>();
  }

  template <typename K>
  void SetIndex(int value) {
    if (std::is_same<T, K>::value) {
      ValueIndex = value;
      return;
    }

    return Permutation<T1...>::template SetIndex<K>(value);
  }

  int GetPermutationId() const {
    return ValueIndex * Super::GetPermutationCount()
         + Super::GetPermutationId();
  }

  void SetFromPermutationId(int permutationId) {
    ValueIndex = (permutationId / Super::GetPermutationCount()) % T::Count;
    Super::SetFromPermutationId(permutationId);
  }

  void GetPermutationDefines(std::string& defines) const {
    defines += "#define ";
    defines += Type::DefineName;
    defines += " ";
    defines += std::to_string(T::Value[ValueIndex]);
    defines += "\r\n";

    Super::GetPermutationDefines(defines);
  }

  int ValueIndex = 0;
};

struct ShaderInfo {
  ShaderInfo() = default;

  ShaderInfo(Name                   name,
             Name                   shaderFilepath,
             Name                   preProcessors,
             Name                   entryPoint,
             EShaderAccessStageFlag shaderType)
      : m_name_(name)
      , m_shaderFilepath_(shaderFilepath)
      , m_preProcessors_(preProcessors)
      , m_entryPoint_(entryPoint)
      , m_shaderType_(shaderType) {}

  void Initialize() {}

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(m_name_.GetNameHash(),
                                       m_shaderFilepath_.GetNameHash(),
                                       m_preProcessors_.GetNameHash(),
                                       m_entryPoint_.GetNameHash(),
                                       m_shaderType_,
                                       m_permutationId_);
    return Hash;
  }

  mutable size_t Hash = 0;

  const Name& GetName() const { return m_name_; }

  const Name& GetShaderFilepath() const { return m_shaderFilepath_; }

  const Name& GetPreProcessors() const { return m_preProcessors_; }

  const Name& GetEntryPoint() const { return m_entryPoint_; }

  const EShaderAccessStageFlag GetShaderType() const { return m_shaderType_; }

  const uint32_t& GetPermutationId() const { return m_permutationId_; }

  const std::vector<Name>& GetIncludeShaderFilePaths() const {
    return m_includeShaderFilePaths_;
  }

  void SetName(const Name& name) {
    m_name_ = name;
    Hash = 0;
  }

  void SetShaderFilepath(const Name& shaderFilepath) {
    m_shaderFilepath_ = shaderFilepath;
    Hash           = 0;
  }

  void SetPreProcessors(const Name& preProcessors) {
    m_preProcessors_ = preProcessors;
    Hash          = 0;
  }

  void SetEntryPoint(const Name& entryPoint) {
    m_entryPoint_ = entryPoint;
    Hash       = 0;
  }

  void SetShaderType(const EShaderAccessStageFlag shaderType) {
    m_shaderType_ = shaderType;
    Hash       = 0;
  }

  void SetPermutationId(const uint32_t permutationId) {
    m_permutationId_ = permutationId;
    Hash          = 0;
  }

  void SetIncludeShaderFilePaths(const std::vector<Name>& paths) {
    m_includeShaderFilePaths_ = paths;
  }

  private:
  Name                   m_name_;
  Name                   m_preProcessors_;
  Name                   m_entryPoint_ = Name("main");
  Name                   m_shaderFilepath_;
  std::vector<Name>      m_includeShaderFilePaths_;
  EShaderAccessStageFlag m_shaderType_    = (EShaderAccessStageFlag)0;
  uint32_t               m_permutationId_ = 0;
};

struct CompiledShader {
  virtual ~CompiledShader() {}
};

struct Shader {
  // TODO: consider renaming 
  static bool                 s_isRunningCheckUpdateShaderThread;
  static std::thread          s_checkUpdateShaderThread;
  static std::vector<Shader*> s_waitForUpdateShaders;
  static std::map<const Shader*, std::vector<size_t>>
      s_connectedPipelineStateHash;

  Shader() {}

  Shader(const ShaderInfo& shaderInfo)
      : m_shaderInfo_(shaderInfo) {}

  virtual ~Shader();

  static void StartAndRunCheckUpdateShaderThread();
  static void ReleaseCheckUpdateShaderThread();

  bool         UpdateShader();
  virtual void Initialize();

  uint64_t   m_timeStamp_ = 0;
  ShaderInfo m_shaderInfo_;

  CompiledShader* GetCompiledShader() const { return m_compiledShader; }

  virtual void SetPermutationId(int32_t permutaitonId) {}

  virtual int32_t GetPermutationId() const { return 0; }

  virtual int32_t GetPermutationCount() const { return 1; }

  virtual void GetPermutationDefines(std::string& result) const {}

  CompiledShader* m_compiledShader = nullptr;
  // TODO: add abstraction (should be related to Vulkan)
};

// TODO: refactor
#define DECLARE_SHADER_WITH_PERMUTATION(ShaderClass, PermutationVariable) \
  public:                                                                 \
  static ShaderInfo   GShaderInfo;                                        \
  static ShaderClass* CreateShader(                                       \
      const ShaderClass::ShaderPermutation& permutation);               \
  using Shader::Shader;                                                   \
  virtual void SetPermutationId(int32_t permutaitonId) override {       \
    PermutationVariable.SetFromPermutationId(permutaitonId);            \
  }                                                                       \
  virtual int32_t GetPermutationId() const override {                     \
    return PermutationVariable.GetPermutationId();                        \
  }                                                                       \
  virtual int32_t GetPermutationCount() const override {                  \
    return PermutationVariable.GetPermutationCount();                     \
  }                                                                       \
  virtual void GetPermutationDefines(std::string& result) const {      \
    PermutationVariable.GetPermutationDefines(result);                 \
  }

struct ShaderForwardPixelShader : public Shader {
  // TODO: currently not used
  // DECLARE_DEFINE(USE_VARIABLE_SHADING_RATE, 0, 1);
  DECLARE_DEFINE(USE_REVERSEZ, 0, 1);

  using ShaderPermutation
      = Permutation</*USE_VARIABLE_SHADING_RATE,*/ USE_REVERSEZ>;
  ShaderPermutation m_permutation_;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderForwardPixelShader, m_permutation_)
};

struct ShaderGBufferVertexShader : public Shader {
  DECLARE_DEFINE(USE_VERTEX_COLOR, 0, 1);
  DECLARE_DEFINE(USE_VERTEX_BITANGENT, 0, 1);
  DECLARE_DEFINE(USE_ALBEDO_TEXTURE, 0, 1);
  DECLARE_DEFINE(USE_SPHERICAL_MAP, 0, 1);

  using ShaderPermutation = Permutation<USE_VERTEX_COLOR,
                                        USE_VERTEX_BITANGENT,
                                        USE_ALBEDO_TEXTURE,
                                        USE_SPHERICAL_MAP>;
  ShaderPermutation m_permutation_;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferVertexShader, m_permutation_)
};

struct ShaderGBufferPixelShader : public Shader {
  DECLARE_DEFINE(USE_VERTEX_COLOR, 0, 1);
  DECLARE_DEFINE(USE_ALBEDO_TEXTURE, 0, 1);
  DECLARE_DEFINE(USE_SRGB_ALBEDO_TEXTURE, 0, 1);
  // TODO: currently not used
  // DECLARE_DEFINE(USE_VARIABLE_SHADING_RATE, 0, 1);
  DECLARE_DEFINE(USE_PBR, 0, 1);

  using ShaderPermutation = Permutation<USE_VERTEX_COLOR,
                                        USE_ALBEDO_TEXTURE,
                                        USE_SRGB_ALBEDO_TEXTURE,
                                        /*USE_VARIABLE_SHADING_RATE,*/
                                        USE_PBR>;
  ShaderPermutation m_permutation_;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferPixelShader, m_permutation_)
};

// TODO: currently not used
// struct ShaderDirectionalLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation = Permutation<USE_SUBPASS, USE_SHADOW_MAP,
//  USE_PBR>; ShaderPermutation m_permutation_;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderDirectionalLightPixelShader,
//                                  m_permutation_)
//};
//
// struct ShaderPointLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation = Permutation<USE_SUBPASS, USE_SHADOW_MAP,
//  USE_PBR>; ShaderPermutation m_permutation_;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderPointLightPixelShader, m_permutation_)
//};
//
// struct ShaderSpotLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_REVERSEZ, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation
//      = Permutation<USE_SUBPASS, USE_SHADOW_MAP, USE_REVERSEZ, USE_PBR>;
//  ShaderPermutation m_permutation_;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderSpotLightPixelShader, m_permutation_)
//};

struct GraphicsPipelineShader {
  Shader* m_vertexShader_   = nullptr;
  Shader* m_geometryShader_ = nullptr;
  Shader* m_pixelShader_    = nullptr;

  size_t GetHash() const;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_H
