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

// Shader Permutation. by using Shader Permutation Define, generate
// m_permutation_ of defines and convert it to m_permutation_ id.
template <typename... T>
class Permutation {
  public:
  // ======= BEGIN: public static methods =====================================

  template <typename K>
  static int s_getDefineCount() {
    return 1;
  }

  static int s_getPermutationCount() { return 1; }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public getters ============================================

  template <typename K>
  int get() const {
    return int();
  }

  template <typename K>
  int getIndex() const {
    return int();
  }

  int getPermutationId() const { return 0; }

  void getPermutationDefines(std::string&) const {}

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setFromPermutationId(int) {}

  template <typename K>
  void setIndex(int) {}

  // ======= END: public setters   ============================================
};

template <typename T, typename... T1>
class Permutation<T, T1...> : public Permutation<T1...> {
  public:
  // ======= BEGIN: public aliases ============================================

  using Type  = T;
  using Super = Permutation<T1...>;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public static methods =====================================

  static int s_getPermutationCount() {
    return T::Count * Super::s_getPermutationCount();
  }

  template <typename K>
  static int s_getDefineCount() {
    if (std::is_same<T, K>::value) {
      return T::Count;
    }

    return Super::template s_getDefineCount<K>();
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public getters ============================================

  template <typename K>
  int get() const {
    if (std::is_same<T, K>::value) {
      return T::Value[ValueIndex];
    }

    return Super::template get<K>();
  }

  template <typename K>
  int getIndex() const {
    if (std::is_same<T, K>::value) {
      return ValueIndex;
    }

    return Super::template getIndex<K>();
  }

  int getPermutationId() const {
    return ValueIndex * Super::s_getPermutationCount()
         + Super::getPermutationId();
  }

  void getPermutationDefines(std::string& defines) const {
    defines += "#define ";
    defines += Type::DefineName;
    defines += " ";
    defines += std::to_string(T::Value[ValueIndex]);
    defines += "\r\n";

    Super::getPermutationDefines(defines);
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  template <typename K>
  void setIndex(int value) {
    if (std::is_same<T, K>::value) {
      ValueIndex = value;
      return;
    }

    return Permutation<T1...>::template setIndex<K>(value);
  }

  void setFromPermutationId(int permutationId) {
    ValueIndex = (permutationId / Super::s_getPermutationCount()) % T::Count;
    Super::setFromPermutationId(permutationId);
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc fields ========================================

  int ValueIndex = 0;

  // ======= END: public misc fields   ========================================
};

struct ShaderInfo {
  // ======= BEGIN: public constructors =======================================

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

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = GETHASH_FROM_INSTANT_STRUCT(m_name_.getNameHash(),
                                          m_shaderFilepath_.getNameHash(),
                                          m_preProcessors_.getNameHash(),
                                          m_entryPoint_.getNameHash(),
                                          m_shaderType_,
                                          m_permutationId_);
    return m_hash_;
  }

  const Name& getName() const { return m_name_; }

  const Name& getShaderFilepath() const { return m_shaderFilepath_; }

  const Name& getPreProcessors() const { return m_preProcessors_; }

  const Name& getEntryPoint() const { return m_entryPoint_; }

  const EShaderAccessStageFlag getShaderType() const { return m_shaderType_; }

  const uint32_t& getPermutationId() const { return m_permutationId_; }

  const std::vector<Name>& getIncludeShaderFilePaths() const {
    return m_includeShaderFilePaths_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setName(const Name& name) {
    m_name_ = name;
    m_hash_ = 0;
  }

  void setShaderFilepath(const Name& shaderFilepath) {
    m_shaderFilepath_ = shaderFilepath;
    m_hash_           = 0;
  }

  void setPreProcessors(const Name& preProcessors) {
    m_preProcessors_ = preProcessors;
    m_hash_          = 0;
  }

  void setEntryPoint(const Name& entryPoint) {
    m_entryPoint_ = entryPoint;
    m_hash_       = 0;
  }

  void setShaderType(const EShaderAccessStageFlag shaderType) {
    m_shaderType_ = shaderType;
    m_hash_       = 0;
  }

  void setPermutationId(const uint32_t permutationId) {
    m_permutationId_ = permutationId;
    m_hash_          = 0;
  }

  void setIncludeShaderFilePaths(const std::vector<Name>& paths) {
    m_includeShaderFilePaths_ = paths;
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void initialize() {}

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  // TODO: consider making private
  mutable size_t m_hash_ = 0;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private misc fields =======================================

  Name                   m_name_;
  Name                   m_preProcessors_;
  Name                   m_entryPoint_ = Name("main");
  Name                   m_shaderFilepath_;
  std::vector<Name>      m_includeShaderFilePaths_;
  EShaderAccessStageFlag m_shaderType_    = (EShaderAccessStageFlag)0;
  uint32_t               m_permutationId_ = 0;

  // ======= END: private misc fields   =======================================
};

struct CompiledShader {
  // ======= BEGIN: public destructor =========================================

  virtual ~CompiledShader() {}

  // ======= END: public destructor   =========================================
};

struct Shader : public std::enable_shared_from_this<Shader> {
  // ======= BEGIN: public static methods =====================================

  static void s_startAndRunCheckUpdateShaderThread();
  static void s_releaseCheckUpdateShaderThread();

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  // TODO: consider renaming
  // static bool s_isRunningCheckUpdateShaderThread;

  // static std::thread s_checkUpdateShaderThread;

  // static std::vector<Shader*> s_waitForUpdateShaders;

  // static std::map<const Shader*, std::vector<size_t>>
  //     s_connectedPipelineStateHash;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public constructors =======================================

  Shader() {}

  Shader(const ShaderInfo& shaderInfo)
      : m_shaderInfo_(shaderInfo) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~Shader();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize();

  virtual int32_t getPermutationId() const { return 0; }

  virtual int32_t getPermutationCount() const { return 1; }

  virtual void getPermutationDefines(std::string& result) const {}

  virtual void setPermutationId(int32_t permutaitonId) {}

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  const std::shared_ptr<CompiledShader>& getCompiledShader() const {
    return m_compiledShader_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool updateShader();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  uint64_t                        m_timeStamp_ = 0;
  ShaderInfo                      m_shaderInfo_;
  std::shared_ptr<CompiledShader> m_compiledShader_ = nullptr;
  // TODO: add abstraction (should be related to Vulkan) - maybe deprecated
  // comment

  // ======= END: public misc fields   ========================================
};

// TODO:
// - consider whether there's better solution for this (templates, etc.)
// - refactor
// #define DECLARE_SHADER_WITH_PERMUTATION(ShaderClass, PermutationVariable)  \
//  public:                                                                  \
//  static ShaderInfo   GShaderInfo;                                         \
//  static ShaderClass* CreateShader(                                        \
//      const ShaderClass::ShaderPermutation& permutation);                  \
//  using Shader::Shader;                                                    \
//  virtual void setPermutationId(int32_t permutaitonId) override {          \
//    PermutationVariable.setFromPermutationId(permutaitonId);               \
//  }                                                                        \
//  virtual int32_t getPermutationId() const override {                      \
//    return PermutationVariable.getPermutationId();                         \
//  }                                                                        \
//  virtual int32_t getPermutationCount() const override {                   \
//    return PermutationVariable.s_getPermutationCount();                    \
//  }                                                                        \
//  virtual void getPermutationDefines(std::string& result) const override { \
//    PermutationVariable.getPermutationDefines(result);                     \
//  }
//
// struct ShaderForwardPixelShader : public Shader {
//  // ======= BEGIN: public nested types
//  =======================================
//
//  // TODO: currently not used
//  // DECLARE_DEFINE(USE_VARIABLE_SHADING_RATE, 0, 1);
//  DECLARE_DEFINE(USE_REVERSEZ, 0, 1);
//
//  // ======= END: public nested types =======================================
//
//  // ======= BEGIN: public aliases
//  ============================================
//
//  using ShaderPermutation
//      = Permutation</*USE_VARIABLE_SHADING_RATE,*/ USE_REVERSEZ>;
//
//  // ======= END: public aliases ============================================
//
//  // ======= BEGIN: public misc fields
//  ========================================
//
//  ShaderPermutation m_permutation_;
//
//  // ======= END: public misc fields ========================================
//
//  // TODO: consider wrap to comment (as other code elements in class)
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderForwardPixelShader, m_permutation_)
//};
//
// struct ShaderGBufferVertexShader : public Shader {
//  // ======= BEGIN: public nested types
//  =======================================
//
//  DECLARE_DEFINE(USE_VERTEX_COLOR, 0, 1);
//  DECLARE_DEFINE(USE_VERTEX_BITANGENT, 0, 1);
//  DECLARE_DEFINE(USE_ALBEDO_TEXTURE, 0, 1);
//  DECLARE_DEFINE(USE_SPHERICAL_MAP, 0, 1);
//
//  // ======= END: public nested types =======================================
//
//  // ======= BEGIN: public aliases
//  ============================================
//
//  using ShaderPermutation = Permutation<USE_VERTEX_COLOR,
//                                        USE_VERTEX_BITANGENT,
//                                        USE_ALBEDO_TEXTURE,
//                                        USE_SPHERICAL_MAP>;
//
//  // ======= END: public aliases ============================================
//
//  // ======= BEGIN: public misc fields
//  ========================================
//
//  ShaderPermutation m_permutation_;
//
//  // ======= END: public misc fields ========================================
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferVertexShader, m_permutation_)
//};
//
// struct ShaderGBufferPixelShader : public Shader {
//  // ======= BEGIN: public nested types
//  =======================================
//
//  DECLARE_DEFINE(USE_VERTEX_COLOR, 0, 1);
//  DECLARE_DEFINE(USE_ALBEDO_TEXTURE, 0, 1);
//  DECLARE_DEFINE(USE_SRGB_ALBEDO_TEXTURE, 0, 1);
//  // TODO: currently not used
//  // DECLARE_DEFINE(USE_VARIABLE_SHADING_RATE, 0, 1);
//  DECLARE_DEFINE(USE_PBR, 0, 1);
//
//  // ======= END: public nested types =======================================
//
//  // ======= BEGIN: public aliases
//  ============================================
//
//  using ShaderPermutation = Permutation<USE_VERTEX_COLOR,
//                                        USE_ALBEDO_TEXTURE,
//                                        USE_SRGB_ALBEDO_TEXTURE,
//                                        /*USE_VARIABLE_SHADING_RATE,*/
//                                        USE_PBR>;
//
//  // ======= END: public aliases ============================================
//
//  // ======= BEGIN: public misc fields
//  ========================================
//
//  ShaderPermutation m_permutation_;
//
//  // ======= END: public misc fields ========================================
//
//  DECLARE_SHADER_WITH_PERMUTATION(ShaderGBufferPixelShader, m_permutation_)
//};

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
  // ======= BEGIN: public getters ============================================

  size_t getHash() const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<Shader> m_vertexShader_   = nullptr;
  std::shared_ptr<Shader> m_geometryShader_ = nullptr;
  std::shared_ptr<Shader> m_pixelShader_    = nullptr;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_H
