#ifndef GAME_ENGINE_SHADER_VK_H
#define GAME_ENGINE_SHADER_VK_H

#include "gfx/rhi/instant_struct.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"
#include "gfx/rhi/vulkan/spirv_util.h"

#include <vulkan/vulkan.h>

#include <filesystem>
#include <iterator>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace game_engine {

// TODO: consider using permutation for dynamic shader compilation

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
class jPermutation {
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
class jPermutation<T, T1...> : public jPermutation<T1...> {
  public:
  using Type  = T;
  using Super = jPermutation<T1...>;

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

    return jPermutation<T1...>::template SetIndex<K>(value);
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

  virtual void SetPermutationId(int32_t InPermutaitonId) {}

  virtual int32_t GetPermutationId() const { return 0; }

  virtual int32_t GetPermutationCount() const { return 1; }

  virtual void GetPermutationDefines(std::string& OutResult) const {}

  // used VkPipelineShaderStageCreateInfo directly instead
  // jCompiledShader* CompiledShader = nullptr;
  // TODO: add abstraction (should be related to Vulkan)
  VkPipelineShaderStageCreateInfo ShaderStage{};
};

// old version
// class Shader {
//  public:
//  Shader(VkShaderStageFlagBits        stage,
//           const std::filesystem::path& shaderFilePath,
//           const std::string&           entryPoint = "main")
//      : m_stage(stage)
//      , m_entryPoint(entryPoint) {
//    auto spirvCode = SpirvUtil::compileHlslFileToSpirv(
//        shaderFilePath, determineShaderKind(stage).value(), entryPoint);
//    createShaderModule(spirvCode);
//  }
//
//  ~Shader() {
//    if (m_shaderModule != VK_NULL_HANDLE) {
//      vkDestroyShaderModule(g_rhi_vk->m_device_, m_shaderModule, nullptr);
//    }
//  }
//
//  VkShaderStageFlagBits getStage() const { return m_stage; }
//
//  VkShaderModule getShaderModule() const { return m_shaderModule; }
//
//  const std::string& getEntryPoint() const { return m_entryPoint; }
//
//  VkPipelineShaderStageCreateInfo getShaderStageInfo() const {
//    return m_shaderStageInfo;
//  }
//
//  private:
//  void createShaderModule(const std::vector<uint32_t>& spirvCode) {
//    VkShaderModuleCreateInfo createInfo{};
//    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
//    createInfo.pCode    = spirvCode.data();
//
//    if (vkCreateShaderModule(
//            g_rhi_vk->m_device_, &createInfo, nullptr, &m_shaderModule)
//        != VK_SUCCESS) {
//      // TODO: log error
//    }
//  }
//
//  static std::optional<shaderc_shader_kind> determineShaderKind(
//      VkShaderStageFlagBits stage) {
//    switch (stage) {
//      case VK_SHADER_STAGE_VERTEX_BIT:
//        return shaderc_vertex_shader;
//      case VK_SHADER_STAGE_FRAGMENT_BIT:
//        return shaderc_fragment_shader;
//      case VK_SHADER_STAGE_GEOMETRY_BIT:
//        return shaderc_geometry_shader;
//      case VK_SHADER_STAGE_COMPUTE_BIT:
//        return shaderc_compute_shader;
//      case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
//        return shaderc_raygen_shader;
//      case VK_SHADER_STAGE_MISS_BIT_KHR:
//        return shaderc_miss_shader;
//      case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
//        return shaderc_closesthit_shader;
//      case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
//        return shaderc_anyhit_shader;
//      default:
//        // Log error or handle unsupported stages
//        return std::nullopt;
//    }
//  }
//
//  VkShaderStageFlagBits m_stage;
//
//  std::string    m_entryPoint;
//  VkShaderModule m_shaderModule = VK_NULL_HANDLE;
//
//  // Compiler shader
//  VkPipelineShaderStageCreateInfo m_shaderStageInfo{};
//};

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
      = jPermutation</*USE_VARIABLE_SHADING_RATE,*/ USE_REVERSEZ>;
  ShaderPermutation Permutation;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderForwardPixelShader, Permutation)
};

struct ShaderGBufferVertexShader : public Shader {
  DECLARE_DEFINE(USE_VERTEX_COLOR, 0, 1);
  DECLARE_DEFINE(USE_VERTEX_BITANGENT, 0, 1);
  DECLARE_DEFINE(USE_ALBEDO_TEXTURE, 0, 1);
  DECLARE_DEFINE(USE_SPHERICAL_MAP, 0, 1);

  using ShaderPermutation = jPermutation<USE_VERTEX_COLOR,
                                         USE_VERTEX_BITANGENT,
                                         USE_ALBEDO_TEXTURE,
                                         USE_SPHERICAL_MAP>;
  ShaderPermutation Permutation;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferVertexShader, Permutation)
};

struct ShaderGBufferPixelShader : public Shader {
  DECLARE_DEFINE(USE_VERTEX_COLOR, 0, 1);
  DECLARE_DEFINE(USE_ALBEDO_TEXTURE, 0, 1);
  DECLARE_DEFINE(USE_SRGB_ALBEDO_TEXTURE, 0, 1);
  // TODO: currently not used
  // DECLARE_DEFINE(USE_VARIABLE_SHADING_RATE, 0, 1);
  DECLARE_DEFINE(USE_PBR, 0, 1);

  using ShaderPermutation = jPermutation<USE_VERTEX_COLOR,
                                         USE_ALBEDO_TEXTURE,
                                         USE_SRGB_ALBEDO_TEXTURE,
                                         /*USE_VARIABLE_SHADING_RATE,*/
                                         USE_PBR>;
  ShaderPermutation Permutation;

  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferPixelShader, Permutation)
};

// TODO: currently not used
// struct ShaderDirectionalLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation = jPermutation<USE_SUBPASS, USE_SHADOW_MAP,
//  USE_PBR>; ShaderPermutation Permutation;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderDirectionalLightPixelShader,
//                                  Permutation)
//};
//
// struct ShaderPointLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation = jPermutation<USE_SUBPASS, USE_SHADOW_MAP,
//  USE_PBR>; ShaderPermutation Permutation;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderPointLightPixelShader, Permutation)
//};
//
// struct ShaderSpotLightPixelShader : public Shader {
//  DECLARE_DEFINE(USE_SUBPASS, 0, 1);
//  DECLARE_DEFINE(USE_SHADOW_MAP, 0, 1);
//  DECLARE_DEFINE(USE_REVERSEZ, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  using ShaderPermutation
//      = jPermutation<USE_SUBPASS, USE_SHADOW_MAP, USE_REVERSEZ, USE_PBR>;
//  ShaderPermutation Permutation;
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderSpotLightPixelShader, Permutation)
//};

struct GraphicsPipelineShader {
  Shader* VertexShader   = nullptr;
  Shader* GeometryShader = nullptr;
  Shader* PixelShader    = nullptr;

  size_t GetHash() const;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_VK_H
