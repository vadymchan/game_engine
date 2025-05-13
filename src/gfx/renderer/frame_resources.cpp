#include "gfx/renderer/frame_resources.h"

#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "gfx/renderer/render_resource_manager.h"
#include "utils/memory/align.h"

namespace game_engine {
namespace gfx {
namespace renderer {

FrameResources::FrameResources(rhi::Device* device, RenderResourceManager* resourceManager)
    : m_device(device)
    , m_resourceManager(resourceManager) {
}

void FrameResources::initialize(uint32_t framesCount) {
  if (m_initialized) {
    return;
  }

  createViewDescriptorSetLayout_();
  createLightDescriptorSetLayouts_();
  createModelMatrixDescriptorSetLayout_();
  createMaterialDescriptorSetLayout_();
  createDefaultTextures_();

  createDefaultSampler_();
  createSamplerDescriptorSet_();

  m_renderTargetsPerFrame.resize(framesCount);

  m_initialized = true;
}

void FrameResources::resize(const math::Dimension2Di& newDimension) {
  m_viewport.x        = 0.0f;
  m_viewport.y        = 0.0f;
  m_viewport.width    = static_cast<float>(newDimension.width());
  m_viewport.height   = static_cast<float>(newDimension.height());
  m_viewport.minDepth = 0.0f;
  m_viewport.maxDepth = 1.0f;

  m_scissor.x      = 0;
  m_scissor.y      = 0;
  m_scissor.width  = newDimension.width();
  m_scissor.height = newDimension.height();

  for (auto& target : m_renderTargetsPerFrame) {
    createRenderTargets_(target, newDimension);
  }
}

void FrameResources::updatePerFrameResources(const RenderContext& context) {
  clearInternalDirtyFlags_();

  updateViewResources_(context);
  updateLightResources_(context);
  updateModelList_(context);

  clearEntityDirtyFlags_(context);
}

void FrameResources::cleanup() {
  m_renderTargetsPerFrame.clear();

  m_viewDescriptorSet             = nullptr;
  m_directionalLightDescriptorSet = nullptr;
  m_pointLightDescriptorSet       = nullptr;
  m_spotLightDescriptorSet        = nullptr;
  m_defaultSamplerDescriptorSet   = nullptr;

  m_viewDescriptorSetLayout     = nullptr;
  m_lightDescriptorSetLayout    = nullptr;
  m_materialDescriptorSetLayout = nullptr;

  m_viewUniformBuffer             = nullptr;
  m_directionalLightUniformBuffer = nullptr;
  m_pointLightUniformBuffer       = nullptr;
  m_spotLightUniformBuffer        = nullptr;

  m_defaultSampler = nullptr;

  m_modelMatrixCache.clear();

  m_sortedModels.clear();
  m_modelsMap.clear();

  m_initialized = false;
}

rhi::DescriptorSet* FrameResources::getOrCreateModelMatrixDescriptorSet(RenderMesh* renderMesh) {
  auto it = m_modelMatrixCache.find(renderMesh);
  if (it != m_modelMatrixCache.end() && it->second.descriptorSet) {
    return it->second.descriptorSet;
  }

  if (!renderMesh->transformMatrixBuffer) {
    GlobalLogger::Log(LogLevel::Warning, "RenderMesh has no model matrix buffer");
    return nullptr;
  }

  auto descriptorSet = m_device->createDescriptorSet(m_modelMatrixDescriptorSetLayout);
  descriptorSet->setUniformBuffer(0, renderMesh->transformMatrixBuffer);

  std::string setKey           = "mesh_model_matrix_set_" + std::to_string(reinterpret_cast<uintptr_t>(renderMesh));
  auto        descriptorSetPtr = m_resourceManager->addDescriptorSet(std::move(descriptorSet), setKey);

  m_modelMatrixCache[renderMesh].descriptorSet = descriptorSetPtr;
  return descriptorSetPtr;
}

rhi::Buffer* FrameResources::getOrCreateMaterialParamBuffer(Material* material) {
  auto it = m_materialParamCache.find(material);
  if (it != m_materialParamCache.end() && it->second.paramBuffer) {
    return it->second.paramBuffer;
  }

  std::string bufferKey = "material_params_" + std::to_string(reinterpret_cast<uintptr_t>(material));

  rhi::BufferDesc bufferDesc;
  bufferDesc.size        = alignConstantBufferSize(sizeof(MaterialParametersData));
  bufferDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
  bufferDesc.type        = rhi::BufferType::Dynamic;
  bufferDesc.debugName   = bufferKey;

  auto buffer    = m_device->createBuffer(bufferDesc);
  auto bufferPtr = m_resourceManager->addBuffer(std::move(buffer), bufferKey);

  MaterialParametersData paramData = {};

  auto colorIt = material->vectorParameters.find("base_color");
  if (colorIt != material->vectorParameters.end()) {
    paramData.baseColor = colorIt->second;
  } else {
    paramData.baseColor = math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f);
  }

  auto metallicIt = material->scalarParameters.find("metallic");
  if (metallicIt != material->scalarParameters.end()) {
    paramData.metallic = metallicIt->second;
  } else {
    paramData.metallic = 0.0f;
  }

  auto roughnessIt = material->scalarParameters.find("roughness");
  if (roughnessIt != material->scalarParameters.end()) {
    paramData.roughness = roughnessIt->second;
  } else {
    paramData.roughness = 1.0f;
  }

  auto opacityIt = material->scalarParameters.find("opacity");
  if (opacityIt != material->scalarParameters.end()) {
    paramData.opacity = opacityIt->second;
  } else {
    paramData.opacity = 1.0f;
  }

  m_device->updateBuffer(bufferPtr, &paramData, sizeof(paramData));

  m_materialParamCache[material].paramBuffer = bufferPtr;

  return bufferPtr;
}

void FrameResources::createViewDescriptorSetLayout_() {
  rhi::DescriptorSetLayoutDesc        viewLayoutDesc;
  rhi::DescriptorSetLayoutBindingDesc viewBindingDesc;
  viewBindingDesc.binding    = 0;
  viewBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
  viewBindingDesc.stageFlags = rhi::ShaderStageFlag::Vertex | rhi::ShaderStageFlag::Fragment;
  viewLayoutDesc.bindings.push_back(viewBindingDesc);

  auto viewSetLayout        = m_device->createDescriptorSetLayout(viewLayoutDesc);
  m_viewDescriptorSetLayout = m_resourceManager->addDescriptorSetLayout(std::move(viewSetLayout), "view_set_layout");
}

void FrameResources::createLightDescriptorSetLayouts_() {
  rhi::DescriptorSetLayoutDesc        lightLayoutDesc;
  rhi::DescriptorSetLayoutBindingDesc lightBindingDesc;
  lightBindingDesc.binding    = 0;
  lightBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
  lightBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
  lightLayoutDesc.bindings.push_back(lightBindingDesc);

  auto lightSetLayout        = m_device->createDescriptorSetLayout(lightLayoutDesc);
  m_lightDescriptorSetLayout = m_resourceManager->addDescriptorSetLayout(std::move(lightSetLayout), "light_set_layout");
}

void FrameResources::createModelMatrixDescriptorSetLayout_() {
  rhi::DescriptorSetLayoutDesc        layoutDesc;
  rhi::DescriptorSetLayoutBindingDesc bindingDesc;
  bindingDesc.binding    = 0;
  bindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
  bindingDesc.stageFlags = rhi::ShaderStageFlag::Vertex;
  layoutDesc.bindings.push_back(bindingDesc);

  auto layout = m_device->createDescriptorSetLayout(layoutDesc);
  m_modelMatrixDescriptorSetLayout
      = m_resourceManager->addDescriptorSetLayout(std::move(layout), "model_matrix_layout");
}

void FrameResources::createMaterialDescriptorSetLayout_() {
  rhi::DescriptorSetLayoutDesc materialLayoutDesc;

  rhi::DescriptorSetLayoutBindingDesc paramsBindingDesc;
  paramsBindingDesc.binding    = 0;
  paramsBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
  paramsBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
  materialLayoutDesc.bindings.push_back(paramsBindingDesc);

  std::vector<std::string> textureNames = {"albedo", "normal_map", "metallic_roughness"};
  uint32_t                 binding      = 1;

  for (const auto& textureName : textureNames) {
    rhi::DescriptorSetLayoutBindingDesc textureBindingDesc;
    textureBindingDesc.binding    = binding++;
    textureBindingDesc.type       = rhi::ShaderBindingType::TextureSrv;
    textureBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
    materialLayoutDesc.bindings.push_back(textureBindingDesc);
  }

  auto materialSetLayout = m_device->createDescriptorSetLayout(materialLayoutDesc);
  m_materialDescriptorSetLayout
      = m_resourceManager->addDescriptorSetLayout(std::move(materialSetLayout), "material_layout");
}

void FrameResources::createDefaultTextures_() {
  // 1x1 white texture (for albedo when missing)
  {
    rhi::TextureDesc texDesc;
    texDesc.width = texDesc.height = 1;
    texDesc.format                 = rhi::TextureFormat::Rgba8;
    texDesc.createFlags            = gfx::rhi::TextureCreateFlag::TransferDst;
    texDesc.initialLayout          = rhi::ResourceLayout::ShaderReadOnly;
    texDesc.debugName              = "default_white_texture";

    auto texture = m_device->createTexture(texDesc);

    // white color
    uint32_t whitePixel = 0xFF'FF'FF'FF;
    m_device->updateTexture(texture.get(), &whitePixel, sizeof(whitePixel));

    m_defaultWhiteTexture = m_resourceManager->addTexture(std::move(texture), "default_white_texture");
  }

  // 1x1 normal map texture (0.5, 0.5, 1.0 for flat normal)
  {
    rhi::TextureDesc texDesc;
    texDesc.width = texDesc.height = 1;
    texDesc.format                 = rhi::TextureFormat::Rgba8;
    texDesc.createFlags            = gfx::rhi::TextureCreateFlag::TransferDst;
    texDesc.initialLayout          = rhi::ResourceLayout::ShaderReadOnly;
    texDesc.debugName              = "default_normal_texture";

    auto texture = m_device->createTexture(texDesc);

    // normal facing up (0.5, 0.5, 1.0, 1.0) encoded as RGBA
    uint32_t normalPixel = 0xFF'80'80'FF;  // RGBA order in memory
    m_device->updateTexture(texture.get(), &normalPixel, sizeof(normalPixel));

    m_defaultNormalTexture = m_resourceManager->addTexture(std::move(texture), "default_normal_texture");
  }

  // 1x1 black texture (for metallic-roughness where black = non-metallic, full rough)
  {
    rhi::TextureDesc texDesc;
    texDesc.width = texDesc.height = 1;
    texDesc.format                 = rhi::TextureFormat::Rgba8;
    texDesc.createFlags            = gfx::rhi::TextureCreateFlag::TransferDst;
    texDesc.initialLayout          = rhi::ResourceLayout::ShaderReadOnly;
    texDesc.debugName              = "default_black_texture";

    auto texture = m_device->createTexture(texDesc);

    // black color
    uint32_t blackPixel = 0xFF'00'00'00;  // Alpha = 1, RGB = 0
    m_device->updateTexture(texture.get(), &blackPixel, sizeof(blackPixel));

    m_defaultBlackTexture = m_resourceManager->addTexture(std::move(texture), "default_black_texture");
  }
}

void FrameResources::createDefaultSampler_() {
  rhi::SamplerDesc samplerDesc;
  samplerDesc.minFilter     = rhi::TextureFilter::Linear;
  samplerDesc.magFilter     = rhi::TextureFilter::Linear;
  samplerDesc.addressModeU  = rhi::TextureAddressMode::Repeat;
  samplerDesc.addressModeV  = rhi::TextureAddressMode::Repeat;
  samplerDesc.addressModeW  = rhi::TextureAddressMode::Repeat;
  samplerDesc.maxAnisotropy = 16.0f;

  auto sampler     = m_device->createSampler(samplerDesc);
  m_defaultSampler = m_resourceManager->addSampler(std::move(sampler), "default_sampler");
}

void FrameResources::createSamplerDescriptorSet_() {
  rhi::DescriptorSetLayoutDesc        samplerLayoutDesc;
  rhi::DescriptorSetLayoutBindingDesc samplerBindingDesc;
  samplerBindingDesc.binding    = 0;
  samplerBindingDesc.type       = rhi::ShaderBindingType::Sampler;
  samplerBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
  samplerLayoutDesc.bindings.push_back(samplerBindingDesc);

  auto samplerSetLayout = m_device->createDescriptorSetLayout(samplerLayoutDesc);
  auto samplerSetLayoutPtr
      = m_resourceManager->addDescriptorSetLayout(std::move(samplerSetLayout), "default_sampler_layout");

  auto samplerDescriptorSet = m_device->createDescriptorSet(samplerSetLayoutPtr);
  samplerDescriptorSet->setSampler(0, m_defaultSampler);

  m_defaultSamplerDescriptorSet
      = m_resourceManager->addDescriptorSet(std::move(samplerDescriptorSet), "default_sampler_descriptor_set");
}

void FrameResources::createRenderTargets_(RenderTargets& targets, const math::Dimension2Di& dimensions) {
  auto width  = dimensions.width() != 0 ? dimensions.width() : 1;
  auto height = dimensions.height() != 0 ? dimensions.height() : 1;

  rhi::TextureDesc colorDesc;
  colorDesc.width       = width;
  colorDesc.height      = height;
  colorDesc.format      = rhi::TextureFormat::Bgra8;
  colorDesc.createFlags = rhi::TextureCreateFlag::Rtv | rhi::TextureCreateFlag::TransferSrc
                        | rhi::TextureCreateFlag::TransferDst /*| rhi::TextureCreateFlag::ShaderResource*/;
  colorDesc.initialLayout = rhi::ResourceLayout::ColorAttachment;
  colorDesc.debugName     = "color_buffer";

  targets.colorBuffer = m_device->createTexture(colorDesc);

  rhi::TextureDesc depthDesc;
  depthDesc.width         = width;
  depthDesc.height        = height;
  depthDesc.format        = rhi::TextureFormat::D24S8;
  depthDesc.createFlags   = rhi::TextureCreateFlag::Dsv;
  depthDesc.initialLayout = rhi::ResourceLayout::DepthStencilAttachment;
  depthDesc.debugName     = "depth_buffer";

  targets.depthBuffer = m_device->createTexture(depthDesc);
}

void FrameResources::updateViewResources_(const RenderContext& context) {
  auto& registry = context.scene->getEntityRegistry();
  auto  view     = registry.view<Transform, Camera, CameraMatrices>();

  if (view.begin() == view.end()) {
    GlobalLogger::Log(LogLevel::Warning, "No main Camera exists!");
    return;
  }

  auto  entity       = *view.begin();
  auto& transform    = view.get<Transform>(entity);
  auto& cameraMatrix = view.get<CameraMatrices>(entity);

  if (!m_viewUniformBuffer) {
    rhi::BufferDesc viewUboDesc;
    viewUboDesc.size = alignConstantBufferSize(sizeof(math::Matrix4f<>) * 3 + sizeof(math::Vector3Df) + sizeof(float));
    viewUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
    viewUboDesc.type        = rhi::BufferType::Dynamic;
    viewUboDesc.debugName   = "view_buffer";

    auto viewBuffer     = m_device->createBuffer(viewUboDesc);
    m_viewUniformBuffer = m_resourceManager->addBuffer(std::move(viewBuffer), "view_buffer");

    // Create descriptor set as well
    auto viewDescriptorSet = m_device->createDescriptorSet(m_viewDescriptorSetLayout);
    viewDescriptorSet->setUniformBuffer(0, m_viewUniformBuffer);
    m_viewDescriptorSet = m_resourceManager->addDescriptorSet(std::move(viewDescriptorSet), "view_descriptor_set");
  }

  struct ViewData {
    math::Matrix4f<> view;
    math::Matrix4f<> projection;
    math::Matrix4f<> viewProjection;
    math::Vector3Df  eyePosition;
    float            padding;
  } viewData;

  viewData.view           = cameraMatrix.view;
  viewData.projection     = cameraMatrix.projection;
  viewData.viewProjection = cameraMatrix.view * cameraMatrix.projection;
  viewData.eyePosition    = transform.translation;
  viewData.padding        = 0.0f;

  m_device->updateBuffer(m_viewUniformBuffer, &viewData, sizeof(viewData));
}

void FrameResources::updateLightResources_(const RenderContext& context) {
  auto& registry = context.scene->getEntityRegistry();

  // Directional Light
  {
    auto dirLightView = registry.view<Light, DirectionalLight>();
    if (dirLightView.begin() != dirLightView.end()) {
      if (!m_directionalLightUniformBuffer) {
        rhi::BufferDesc lightUboDesc;
        lightUboDesc.size        = alignConstantBufferSize(sizeof(float) * 8);
        lightUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
        lightUboDesc.type        = rhi::BufferType::Dynamic;
        lightUboDesc.debugName   = "directional_light_buffer";

        auto lightBuffer = m_device->createBuffer(lightUboDesc);
        m_directionalLightUniformBuffer
            = m_resourceManager->addBuffer(std::move(lightBuffer), "directional_light_buffer");

        auto lightDescriptorSet = m_device->createDescriptorSet(m_lightDescriptorSetLayout);
        lightDescriptorSet->setUniformBuffer(0, m_directionalLightUniformBuffer);
        m_directionalLightDescriptorSet
            = m_resourceManager->addDescriptorSet(std::move(lightDescriptorSet), "directional_light_descriptor");
      }

      auto  entity   = *dirLightView.begin();
      auto& light    = dirLightView.get<Light>(entity);
      auto& dirLight = dirLightView.get<DirectionalLight>(entity);

      struct DirectionalLightData {
        float color[3];
        float intensity;
        float direction[3];
        float padding;
      } lightData;

      lightData.color[0]     = light.color.x();
      lightData.color[1]     = light.color.y();
      lightData.color[2]     = light.color.z();
      lightData.intensity    = light.intensity;
      lightData.direction[0] = dirLight.direction.x();
      lightData.direction[1] = dirLight.direction.y();
      lightData.direction[2] = dirLight.direction.z();
      lightData.padding      = 0.0f;

      m_device->updateBuffer(m_directionalLightUniformBuffer, &lightData, sizeof(lightData));
    }
  }

  // Point Light
  {
    auto pointLightView = registry.view<Light, PointLight, Transform>();
    if (pointLightView.begin() != pointLightView.end()) {
      if (!m_pointLightUniformBuffer) {
        rhi::BufferDesc lightUboDesc;
        lightUboDesc.size        = alignConstantBufferSize(sizeof(float) * 8);
        lightUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
        lightUboDesc.type        = rhi::BufferType::Dynamic;
        lightUboDesc.debugName   = "point_light_buffer";

        auto lightBuffer          = m_device->createBuffer(lightUboDesc);
        m_pointLightUniformBuffer = m_resourceManager->addBuffer(std::move(lightBuffer), "point_light_buffer");

        auto lightDescriptorSet = m_device->createDescriptorSet(m_lightDescriptorSetLayout);
        lightDescriptorSet->setUniformBuffer(0, m_pointLightUniformBuffer);
        m_pointLightDescriptorSet
            = m_resourceManager->addDescriptorSet(std::move(lightDescriptorSet), "point_light_descriptor");
      }

      auto  entity     = *pointLightView.begin();
      auto& light      = pointLightView.get<Light>(entity);
      auto& pointLight = pointLightView.get<PointLight>(entity);
      auto& transform  = pointLightView.get<Transform>(entity);

      struct PointLightData {
        float color[3];
        float intensity;
        float range;
        float position[3];
      } lightData;

      lightData.color[0]    = light.color.x();
      lightData.color[1]    = light.color.y();
      lightData.color[2]    = light.color.z();
      lightData.intensity   = light.intensity;
      lightData.range       = pointLight.range;
      lightData.position[0] = transform.translation.x();
      lightData.position[1] = transform.translation.y();
      lightData.position[2] = transform.translation.z();

      m_device->updateBuffer(m_pointLightUniformBuffer, &lightData, sizeof(lightData));
    }
  }

  // Spot Light
  {
    auto spotLightView = registry.view<Light, SpotLight, Transform>();
    if (spotLightView.begin() != spotLightView.end()) {
      if (!m_spotLightUniformBuffer) {
        rhi::BufferDesc lightUboDesc;
        lightUboDesc.size        = alignConstantBufferSize(sizeof(float) * 16);
        lightUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
        lightUboDesc.type        = rhi::BufferType::Dynamic;
        lightUboDesc.debugName   = "spot_light_buffer";

        auto lightBuffer         = m_device->createBuffer(lightUboDesc);
        m_spotLightUniformBuffer = m_resourceManager->addBuffer(std::move(lightBuffer), "spot_light_buffer");

        auto lightDescriptorSet = m_device->createDescriptorSet(m_lightDescriptorSetLayout);
        lightDescriptorSet->setUniformBuffer(0, m_spotLightUniformBuffer);
        m_spotLightDescriptorSet
            = m_resourceManager->addDescriptorSet(std::move(lightDescriptorSet), "spot_light_descriptor");
      }

      auto  entity    = *spotLightView.begin();
      auto& light     = spotLightView.get<Light>(entity);
      auto& spotLight = spotLightView.get<SpotLight>(entity);
      auto& transform = spotLightView.get<Transform>(entity);

      struct SpotLightData {
        float color[3];
        float intensity;
        float range;
        float innerConeAngle;
        float outerConeAngle;
        float padding1;
        float position[3];
        float padding2;
        float direction[3];
        float padding3;
      } lightData;

      lightData.color[0]       = light.color.x();
      lightData.color[1]       = light.color.y();
      lightData.color[2]       = light.color.z();
      lightData.intensity      = light.intensity;
      lightData.range          = spotLight.range;
      lightData.innerConeAngle = spotLight.innerConeAngle;
      lightData.outerConeAngle = spotLight.outerConeAngle;
      lightData.padding1       = 0.0f;
      lightData.position[0]    = transform.translation.x();
      lightData.position[1]    = transform.translation.y();
      lightData.position[2]    = transform.translation.z();
      lightData.padding2       = 0.0f;

      lightData.direction[0] = transform.rotation.x();
      lightData.direction[1] = transform.rotation.y();
      lightData.direction[2] = transform.rotation.z();
      lightData.padding3     = 0.0f;

      m_device->updateBuffer(m_spotLightUniformBuffer, &lightData, sizeof(lightData));
    }
  }
}

void FrameResources::updateModelList_(const RenderContext& context) {
  bool                             needRebuildRenderArray = false;
  std::unordered_set<entt::entity> currentEntityIds;

  auto& registry = context.scene->getEntityRegistry();
  auto  view     = registry.view<Transform, RenderModel*>();

  for (auto entity : view) {
    currentEntityIds.insert(entity);
    auto& transform   = view.get<Transform>(entity);
    auto& renderModel = view.get<RenderModel*>(entity);

    auto it = m_modelsMap.find(entity);
    if (it != m_modelsMap.end()) {
      if (transform.isDirty) {
        it->second.transform   = transform;
        it->second.modelMatrix = calculateTransformMatrix(transform);
        it->second.isDirty     = true;
      }
    } else {
      ModelInstance instance;
      instance.model       = renderModel;
      instance.transform   = transform;
      instance.modelMatrix = calculateTransformMatrix(transform);
      instance.entityId    = entity;
      instance.isDirty     = true;

      if (!renderModel->renderMeshes.empty() && renderModel->renderMeshes[0]->material) {
        instance.materialId = reinterpret_cast<uintptr_t>(renderModel->renderMeshes[0]->material);
      }

      m_modelsMap[entity]    = instance;
      needRebuildRenderArray = true;
    }
  }

  for (auto it = m_modelsMap.begin(); it != m_modelsMap.end();) {
    if (!currentEntityIds.contains(it->first)) {
      it                     = m_modelsMap.erase(it);
      needRebuildRenderArray = true;
    } else {
      ++it;
    }
  }

  if (needRebuildRenderArray) {
    m_sortedModels.clear();
    m_sortedModels.reserve(m_modelsMap.size());

    for (auto& pair : m_modelsMap) {
      m_sortedModels.push_back(&pair.second);
    }

    sortModelsByMaterial_();
  }
}

void FrameResources::sortModelsByMaterial_() {
  std::sort(m_sortedModels.begin(), m_sortedModels.end(), [](const ModelInstance* a, const ModelInstance* b) {
    return a->materialId < b->materialId;
  });
}

void FrameResources::clearInternalDirtyFlags_() {
  for (auto& instance : m_sortedModels) {
    instance->isDirty = false;
  }
}

void FrameResources::clearEntityDirtyFlags_(const RenderContext& context) {
  if (context.scene) {
    auto& registry = context.scene->getEntityRegistry();
    auto  view     = registry.view<Transform>();

    for (auto entity : view) {
      auto& transform = view.get<Transform>(entity);
      if (transform.isDirty) {
        transform.isDirty = false;
      }
    }
  }
}

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine