#ifndef GAME_ENGINE_SHADER_H
#define GAME_ENGINE_SHADER_H

#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h" // TODO: check and remove if not needed
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

// Shader Permutation. by using Shader Permutation Define, generate permutation
// of defines and convert it to permutation id.
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

  void GetPermutationDefines(std::string& OutDefines) const {
    OutDefines += "#define ";
    OutDefines += Type::DefineName;
    OutDefines += " ";
    OutDefines += std::to_string(T::Value[ValueIndex]);
    OutDefines += "\r\n";

    Super::GetPermutationDefines(OutDefines);
  }

  int ValueIndex = 0;
};

struct ShaderInfo {
  ShaderInfo() = default;

  ShaderInfo(Name                   InName,
             Name                   InShaderFilepath,
             Name                   InPreProcessors,
             Name                   InEntryPoint,
             EShaderAccessStageFlag InShaderType)
      : name(InName)
      , ShaderFilepath(InShaderFilepath)
      , PreProcessors(InPreProcessors)
      , EntryPoint(InEntryPoint)
      , ShaderType(InShaderType) {}

  void Initialize() {}

  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GETHASH_FROM_INSTANT_STRUCT(name.GetNameHash(),
                                       ShaderFilepath.GetNameHash(),
                                       PreProcessors.GetNameHash(),
                                       EntryPoint.GetNameHash(),
                                       ShaderType,
                                       PermutationId);
    return Hash;
  }

  mutable size_t Hash = 0;

  const Name& GetName() const { return name; }

  const Name& GetShaderFilepath() const { return ShaderFilepath; }

  const Name& GetPreProcessors() const { return PreProcessors; }

  const Name& GetEntryPoint() const { return EntryPoint; }

  const EShaderAccessStageFlag GetShaderType() const { return ShaderType; }

  const uint32_t& GetPermutationId() const { return PermutationId; }

  const std::vector<Name>& GetIncludeShaderFilePaths() const {
    return IncludeShaderFilePaths;
  }

  void SetName(const Name& InName) {
    name = InName;
    Hash = 0;
  }

  void SetShaderFilepath(const Name& InShaderFilepath) {
    ShaderFilepath = InShaderFilepath;
    Hash           = 0;
  }

  void SetPreProcessors(const Name& InPreProcessors) {
    PreProcessors = InPreProcessors;
    Hash          = 0;
  }

  void SetEntryPoint(const Name& InEntryPoint) {
    EntryPoint = InEntryPoint;
    Hash       = 0;
  }

  void SetShaderType(const EShaderAccessStageFlag InShaderType) {
    ShaderType = InShaderType;
    Hash       = 0;
  }

  void SetPermutationId(const uint32_t InPermutationId) {
    PermutationId = InPermutationId;
    Hash          = 0;
  }

  void SetIncludeShaderFilePaths(const std::vector<Name>& InPaths) {
    IncludeShaderFilePaths = InPaths;
  }

  private:
  Name                   name;
  Name                   PreProcessors;
  Name                   EntryPoint = Name("main");
  Name                   ShaderFilepath;
  std::vector<Name>      IncludeShaderFilePaths;
  EShaderAccessStageFlag ShaderType    = (EShaderAccessStageFlag)0;
  uint32_t               PermutationId = 0;
};

struct jCompiledShader {
  virtual ~jCompiledShader() {}
};

struct Shader {
  static bool                 IsRunningCheckUpdateShaderThread;
  static std::thread          CheckUpdateShaderThread;
  static std::vector<Shader*> WaitForUpdateShaders;
  static std::map<const Shader*, std::vector<size_t>>
      gConnectedPipelineStateHash;

  Shader() {}

  Shader(const ShaderInfo& shaderInfo)
      : shaderInfo(shaderInfo) {}

  virtual ~Shader();

  static void StartAndRunCheckUpdateShaderThread();
  static void ReleaseCheckUpdateShaderThread();

  bool         UpdateShader();
  virtual void Initialize();

  uint64_t   TimeStamp = 0;
  ShaderInfo shaderInfo;

  jCompiledShader* GetCompiledShader() const { return CompiledShader; }

  virtual void SetPermutationId(int32_t InPermutaitonId) {}

  virtual int32_t GetPermutationId() const { return 0; }

  virtual int32_t GetPermutationCount() const { return 1; }

  virtual void GetPermutationDefines(std::string& OutResult) const {}

  jCompiledShader* CompiledShader = nullptr;
  // TODO: add abstraction (should be related to Vulkan)

};



#define DECLARE_SHADER_WITH_PERMUTATION(ShaderClass, PermutationVariable) \
  public:                                                                 \
  static ShaderInfo   GShaderInfo;                                        \
  static ShaderClass* CreateShader(                                       \
      const ShaderClass::ShaderPermutation& InPermutation);               \
  using Shader::Shader;                                                   \
  virtual void SetPermutationId(int32_t InPermutaitonId) override {       \
    PermutationVariable.SetFromPermutationId(InPermutaitonId);            \
  }                                                                       \
  virtual int32_t GetPermutationId() const override {                     \
    return PermutationVariable.GetPermutationId();                        \
  }                                                                       \
  virtual int32_t GetPermutationCount() const override {                  \
    return PermutationVariable.GetPermutationCount();                     \
  }                                                                       \
  virtual void GetPermutationDefines(std::string& OutResult) const {      \
    PermutationVariable.GetPermutationDefines(OutResult);                 \
  }

struct ShaderForwardPixelShader : public Shader {
  // TODO: currently not used
  // DECLARE_DEFINE(USE_VARIABLE_SHADING_RATE, 0, 1);
  DECLARE_DEFINE(USE_REVERSEZ, 0, 1);

  using ShaderPermutation
      = Permutation</*USE_VARIABLE_SHADING_RATE,*/ USE_REVERSEZ>;
  ShaderPermutation permutation;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderForwardPixelShader, permutation)
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
  ShaderPermutation permutation;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferVertexShader, permutation)
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
  ShaderPermutation permutation;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferPixelShader, permutation)
};

// TODO: currently not used
// struct ShaderDirectionalLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation = Permutation<USE_SUBPASS, USE_SHADOW_MAP,
//  USE_PBR>; ShaderPermutation permutation;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderDirectionalLightPixelShader,
//                                  permutation)
//};
//
// struct ShaderPointLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation = Permutation<USE_SUBPASS, USE_SHADOW_MAP,
//  USE_PBR>; ShaderPermutation permutation;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderPointLightPixelShader, permutation)
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
//  ShaderPermutation permutation;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderSpotLightPixelShader, permutation)
//};

struct GraphicsPipelineShader {
  Shader* VertexShader   = nullptr;
  Shader* GeometryShader = nullptr;
  Shader* PixelShader    = nullptr;

  size_t GetHash() const;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_H
