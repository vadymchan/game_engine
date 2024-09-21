#include "gfx/rhi/vulkan/rhi_type_vk.h"

namespace game_engine {

// clang-format off

// ======= BEGIN: Texture filter mapping ======================================

static const std::unordered_map<ETextureFilter, VkFilter> filterMapping = {
  { ETextureFilter::NEAREST,                VK_FILTER_NEAREST },
  { ETextureFilter::LINEAR,                 VK_FILTER_LINEAR  },
  { ETextureFilter::NEAREST_MIPMAP_NEAREST, VK_FILTER_NEAREST },
  { ETextureFilter::LINEAR_MIPMAP_NEAREST,  VK_FILTER_LINEAR  },
  { ETextureFilter::NEAREST_MIPMAP_LINEAR,  VK_FILTER_NEAREST },
  { ETextureFilter::LINEAR_MIPMAP_LINEAR,   VK_FILTER_LINEAR  }
};

// ======= END: Texture filter mapping   ======================================

// ======= BEGIN: Texture address mode mapping =================================

static const std::unordered_map<ETextureAddressMode, VkSamplerAddressMode> addressModeMapping = {
  { ETextureAddressMode::REPEAT,               VK_SAMPLER_ADDRESS_MODE_REPEAT               },
  { ETextureAddressMode::MIRRORED_REPEAT,      VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT      },
  { ETextureAddressMode::CLAMP_TO_EDGE,        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE        },
  { ETextureAddressMode::CLAMP_TO_BORDER,      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER      },
  { ETextureAddressMode::MIRROR_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE }
};

// ======= END: Texture address mode mapping   =================================

// ======= BEGIN: Mipmap mode mapping =========================================

static const std::unordered_map<ETextureFilter, VkSamplerMipmapMode> mipmapModeMapping = {
  { ETextureFilter::NEAREST,                VK_SAMPLER_MIPMAP_MODE_NEAREST },
  { ETextureFilter::LINEAR,                 VK_SAMPLER_MIPMAP_MODE_NEAREST },
  { ETextureFilter::NEAREST_MIPMAP_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST },
  { ETextureFilter::LINEAR_MIPMAP_NEAREST,  VK_SAMPLER_MIPMAP_MODE_NEAREST },
  { ETextureFilter::NEAREST_MIPMAP_LINEAR,  VK_SAMPLER_MIPMAP_MODE_LINEAR  },
  { ETextureFilter::LINEAR_MIPMAP_LINEAR,   VK_SAMPLER_MIPMAP_MODE_LINEAR  }
};

// ======= END: Mipmap mode mapping   =========================================

// ======= BEGIN: Texture format mapping ======================================

static const std::unordered_map<ETextureFormat, VkFormat> formatMapping = {
  { ETextureFormat::RGB8,       VK_FORMAT_R8G8B8_UNORM            },
  { ETextureFormat::RGB32F,     VK_FORMAT_R32G32B32_SFLOAT        },
  { ETextureFormat::RGB16F,     VK_FORMAT_R16G16B16_SFLOAT        },
  { ETextureFormat::R11G11B10F, VK_FORMAT_B10G11R11_UFLOAT_PACK32 },
  { ETextureFormat::RGBA8,      VK_FORMAT_R8G8B8A8_UNORM          },
  { ETextureFormat::RGBA16F,    VK_FORMAT_R16G16B16A16_SFLOAT     },
  { ETextureFormat::RGBA32F,    VK_FORMAT_R32G32B32A32_SFLOAT     },
  { ETextureFormat::RGBA8SI,    VK_FORMAT_R8G8B8A8_SINT           },
  { ETextureFormat::RGBA8UI,    VK_FORMAT_R8G8B8A8_UINT           },
  { ETextureFormat::BGRA8,      VK_FORMAT_B8G8R8A8_UNORM          },
  { ETextureFormat::R8,         VK_FORMAT_R8_UNORM                },
  { ETextureFormat::R16F,       VK_FORMAT_R16_SFLOAT              },
  { ETextureFormat::R32F,       VK_FORMAT_R32_SFLOAT              },
  { ETextureFormat::R8UI,       VK_FORMAT_R8_UINT                 },
  { ETextureFormat::R32UI,      VK_FORMAT_R32_UINT                },
  { ETextureFormat::RG8,        VK_FORMAT_R8G8_UNORM              },
  { ETextureFormat::RG16F,      VK_FORMAT_R16G16_SFLOAT           },
  { ETextureFormat::RG32F,      VK_FORMAT_R32G32_SFLOAT           },
  { ETextureFormat::D16,        VK_FORMAT_D16_UNORM               },
  { ETextureFormat::D16_S8,     VK_FORMAT_D16_UNORM_S8_UINT       },
  { ETextureFormat::D24,        VK_FORMAT_X8_D24_UNORM_PACK32     },
  { ETextureFormat::D24_S8,     VK_FORMAT_D24_UNORM_S8_UINT       },
  { ETextureFormat::D32,        VK_FORMAT_D32_SFLOAT              },
  { ETextureFormat::D32_S8,     VK_FORMAT_D32_SFLOAT_S8_UINT      },
  { ETextureFormat::BC1_UNORM,  VK_FORMAT_BC1_RGBA_UNORM_BLOCK    },
  { ETextureFormat::BC2_UNORM,  VK_FORMAT_BC2_UNORM_BLOCK         },
  { ETextureFormat::BC3_UNORM,  VK_FORMAT_BC3_UNORM_BLOCK         },
  { ETextureFormat::BC4_UNORM,  VK_FORMAT_BC4_UNORM_BLOCK         },
  { ETextureFormat::BC4_SNORM,  VK_FORMAT_BC4_SNORM_BLOCK         },
  { ETextureFormat::BC5_UNORM,  VK_FORMAT_BC5_UNORM_BLOCK         },
  { ETextureFormat::BC5_SNORM,  VK_FORMAT_BC5_SNORM_BLOCK         },
  { ETextureFormat::BC6H_UF16,  VK_FORMAT_BC6H_UFLOAT_BLOCK       },
  { ETextureFormat::BC6H_SF16,  VK_FORMAT_BC6H_SFLOAT_BLOCK       },
  { ETextureFormat::BC7_UNORM,  VK_FORMAT_BC7_UNORM_BLOCK         }
};

// ======= END: Texture format mapping   ======================================

// ======= BEGIN: Primitive topology mapping ==================================

static const std::unordered_map<EPrimitiveType, VkPrimitiveTopology> topologyMapping = {
  { EPrimitiveType::POINTS,                   VK_PRIMITIVE_TOPOLOGY_POINT_LIST                    },
  { EPrimitiveType::LINES,                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST                     },
  { EPrimitiveType::LINES_ADJACENCY,          VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY      },
  { EPrimitiveType::LINE_STRIP_ADJACENCY,     VK_PRIMITIVE_TOPOLOGY_LINE_STRIP                    },
  { EPrimitiveType::TRIANGLES,                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST                 },
  { EPrimitiveType::TRIANGLES_ADJACENCY,      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY  },
  { EPrimitiveType::TRIANGLE_STRIP_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY },
  { EPrimitiveType::TRIANGLE_STRIP,           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP                }
};

// ======= END: Primitive topology mapping   ==================================

// ======= BEGIN: Vertex input rate mapping ===================================

static const std::unordered_map<EVertexInputRate, VkVertexInputRate> inputRateMapping = {
  { EVertexInputRate::VERTEX,   VK_VERTEX_INPUT_RATE_VERTEX   },
  { EVertexInputRate::INSTANCE, VK_VERTEX_INPUT_RATE_INSTANCE }
};

// ======= END: Vertex input rate mapping   ===================================

// ======= BEGIN: Polygon mode mapping ========================================

static const std::unordered_map<EPolygonMode, VkPolygonMode> polygonModeMapping = {
  { EPolygonMode::POINT, VK_POLYGON_MODE_POINT },
  { EPolygonMode::LINE,  VK_POLYGON_MODE_LINE  },
  { EPolygonMode::FILL,  VK_POLYGON_MODE_FILL  }
};

// ======= END: Polygon mode mapping   ========================================

// ======= BEGIN: Front face mapping ==========================================

static const std::unordered_map<EFrontFace, VkFrontFace> frontFaceMapping = {
  { EFrontFace::CW,  VK_FRONT_FACE_CLOCKWISE         },
  { EFrontFace::CCW, VK_FRONT_FACE_COUNTER_CLOCKWISE }
};

// ======= END: Front face mapping   ==========================================

// ======= BEGIN: Stencil op mapping ==========================================

static const std::unordered_map<EStencilOp, VkStencilOp> stencilOpMapping = {
  { EStencilOp::KEEP,      VK_STENCIL_OP_KEEP                },
  { EStencilOp::ZERO,      VK_STENCIL_OP_ZERO                },
  { EStencilOp::REPLACE,   VK_STENCIL_OP_REPLACE             },
  { EStencilOp::INCR,      VK_STENCIL_OP_INCREMENT_AND_CLAMP },
  { EStencilOp::INCR_WRAP, VK_STENCIL_OP_INCREMENT_AND_WRAP  },
  { EStencilOp::DECR,      VK_STENCIL_OP_DECREMENT_AND_CLAMP },
  { EStencilOp::DECR_WRAP, VK_STENCIL_OP_DECREMENT_AND_WRAP  },
  { EStencilOp::INVERT,    VK_STENCIL_OP_INVERT              }
};

// ======= END: Stencil op mapping   ==========================================

// ======= BEGIN: Compare op mapping ==========================================

static const std::unordered_map<ECompareOp, VkCompareOp> compareOpMapping = {
  { ECompareOp::NEVER,    VK_COMPARE_OP_NEVER            },
  { ECompareOp::LESS,     VK_COMPARE_OP_LESS             },
  { ECompareOp::EQUAL,    VK_COMPARE_OP_EQUAL            },
  { ECompareOp::LEQUAL,   VK_COMPARE_OP_LESS_OR_EQUAL    },
  { ECompareOp::GREATER,  VK_COMPARE_OP_GREATER          },
  { ECompareOp::NOTEQUAL, VK_COMPARE_OP_NOT_EQUAL        },
  { ECompareOp::GEQUAL,   VK_COMPARE_OP_GREATER_OR_EQUAL },
  { ECompareOp::ALWAYS,   VK_COMPARE_OP_ALWAYS           }
};

// ======= END: Compare op mapping   ==========================================

// ======= BEGIN: Blend factor mapping ========================================

static const std::unordered_map<EBlendFactor, VkBlendFactor> blendFactorMapping = {
  { EBlendFactor::ZERO,                     VK_BLEND_FACTOR_ZERO                     },
  { EBlendFactor::ONE,                      VK_BLEND_FACTOR_ONE                      },
  { EBlendFactor::SRC_COLOR,                VK_BLEND_FACTOR_SRC_COLOR                },
  { EBlendFactor::ONE_MINUS_SRC_COLOR,      VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR      },
  { EBlendFactor::DST_COLOR,                VK_BLEND_FACTOR_DST_COLOR                },
  { EBlendFactor::ONE_MINUS_DST_COLOR,      VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR      },
  { EBlendFactor::SRC_ALPHA,                VK_BLEND_FACTOR_SRC_ALPHA                },
  { EBlendFactor::ONE_MINUS_SRC_ALPHA,      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA      },
  { EBlendFactor::DST_ALPHA,                VK_BLEND_FACTOR_DST_ALPHA                },
  { EBlendFactor::ONE_MINUS_DST_ALPHA,      VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA      },
  { EBlendFactor::CONSTANT_COLOR,           VK_BLEND_FACTOR_CONSTANT_COLOR           },
  { EBlendFactor::ONE_MINUS_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR },
  { EBlendFactor::CONSTANT_ALPHA,           VK_BLEND_FACTOR_CONSTANT_ALPHA           },
  { EBlendFactor::ONE_MINUS_CONSTANT_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA },
  { EBlendFactor::SRC_ALPHA_SATURATE,       VK_BLEND_FACTOR_SRC_ALPHA_SATURATE       }
};

// ======= END: Blend factor mapping   ========================================

// ======= BEGIN: Blend op mapping ============================================

static const std::unordered_map<EBlendOp, VkBlendOp> blendOpMapping = {
  { EBlendOp::ADD,              VK_BLEND_OP_ADD              },
  { EBlendOp::SUBTRACT,         VK_BLEND_OP_SUBTRACT         },
  { EBlendOp::REVERSE_SUBTRACT, VK_BLEND_OP_REVERSE_SUBTRACT },
  { EBlendOp::MIN_VALUE,        VK_BLEND_OP_MIN              },
  { EBlendOp::MAX_VALUE,        VK_BLEND_OP_MAX              }
};

// ======= END: Blend op mapping   ============================================

// ======= BEGIN: Image layout mapping ========================================

static const std::unordered_map<EResourceLayout, VkImageLayout> imageLayoutMapping = {
  { EResourceLayout::UNDEFINED,                          VK_IMAGE_LAYOUT_UNDEFINED                                  },
  { EResourceLayout::GENERAL,                            VK_IMAGE_LAYOUT_GENERAL                                    },
  { EResourceLayout::UAV,                                VK_IMAGE_LAYOUT_GENERAL                                    },
  { EResourceLayout::COLOR_ATTACHMENT,                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                   },
  { EResourceLayout::DEPTH_STENCIL_ATTACHMENT,           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL           },
  { EResourceLayout::DEPTH_STENCIL_READ_ONLY,            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL            },
  { EResourceLayout::SHADER_READ_ONLY,                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                   },
  { EResourceLayout::TRANSFER_SRC,                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL                       },
  { EResourceLayout::TRANSFER_DST,                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL                       },
  { EResourceLayout::PREINITIALIZED,                     VK_IMAGE_LAYOUT_PREINITIALIZED                             },
  { EResourceLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL },
  { EResourceLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL },
  { EResourceLayout::DEPTH_ATTACHMENT,                   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL                   },
  { EResourceLayout::DEPTH_READ_ONLY,                    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL                    },
  { EResourceLayout::STENCIL_ATTACHMENT,                 VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL                 },
  { EResourceLayout::STENCIL_READ_ONLY,                  VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL                  },
  { EResourceLayout::PRESENT_SRC,                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                            },
  { EResourceLayout::SHARED_PRESENT,                     VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR                         },
  { EResourceLayout::SHADING_RATE_NV,                    VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV                    },
  { EResourceLayout::FRAGMENT_DENSITY_MAP_EXT,           VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT           },
  { EResourceLayout::READ_ONLY,                          VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR                      },
  { EResourceLayout::ATTACHMENT,                         VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR                     },
  { EResourceLayout::ACCELERATION_STRUCTURE,             VK_IMAGE_LAYOUT_GENERAL                                    }
};

// ======= END: Image layout mapping   ========================================

// ======= BEGIN: Pipeline dynamic state mapping ==============================

static const std::unordered_map<EPipelineDynamicState, VkDynamicState> dynamicStateMapping = {
  { EPipelineDynamicState::VIEWPORT,                            VK_DYNAMIC_STATE_VIEWPORT                            },
  { EPipelineDynamicState::SCISSOR,                             VK_DYNAMIC_STATE_SCISSOR                             },
  { EPipelineDynamicState::LINE_WIDTH,                          VK_DYNAMIC_STATE_LINE_WIDTH                          },
  { EPipelineDynamicState::DEPTH_BIAS,                          VK_DYNAMIC_STATE_DEPTH_BIAS                          },
  { EPipelineDynamicState::BLEND_CONSTANTS,                     VK_DYNAMIC_STATE_BLEND_CONSTANTS                     },
  { EPipelineDynamicState::DEPTH_BOUNDS,                        VK_DYNAMIC_STATE_DEPTH_BOUNDS                        },
  { EPipelineDynamicState::STENCIL_COMPARE_MASK,                VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK                },
  { EPipelineDynamicState::STENCIL_WRITE_MASK,                  VK_DYNAMIC_STATE_STENCIL_WRITE_MASK                  },
  { EPipelineDynamicState::STENCIL_REFERENCE,                   VK_DYNAMIC_STATE_STENCIL_REFERENCE                   },
  { EPipelineDynamicState::CULL_MODE,                           VK_DYNAMIC_STATE_CULL_MODE                           },
  { EPipelineDynamicState::FRONT_FACE,                          VK_DYNAMIC_STATE_FRONT_FACE                          },
  { EPipelineDynamicState::PRIMITIVE_TOPOLOGY,                  VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY                  },
  { EPipelineDynamicState::VIEWPORT_WITH_COUNT,                 VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT                 },
  { EPipelineDynamicState::SCISSOR_WITH_COUNT,                  VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT                  },
  { EPipelineDynamicState::VERTEX_INPUT_BINDING_STRIDE,         VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE         },
  { EPipelineDynamicState::DEPTH_TEST_ENABLE,                   VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE                   },
  { EPipelineDynamicState::DEPTH_WRITE_ENABLE,                  VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE                  },
  { EPipelineDynamicState::DEPTH_COMPARE_OP,                    VK_DYNAMIC_STATE_DEPTH_COMPARE_OP                    },
  { EPipelineDynamicState::DEPTH_BOUNDS_TEST_ENABLE,            VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE            },
  { EPipelineDynamicState::STENCIL_TEST_ENABLE,                 VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE                 },
  { EPipelineDynamicState::STENCIL_OP,                          VK_DYNAMIC_STATE_STENCIL_OP                          },
  { EPipelineDynamicState::RASTERIZER_DISCARD_ENABLE,           VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE           },
  { EPipelineDynamicState::DEPTH_BIAS_ENABLE,                   VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE                   },
  { EPipelineDynamicState::PRIMITIVE_RESTART_ENABLE,            VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE            },
  { EPipelineDynamicState::VIEWPORT_W_SCALING_NV,               VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV               },
  { EPipelineDynamicState::DISCARD_RECTANGLE_EXT,               VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT               },
  { EPipelineDynamicState::SAMPLE_LOCATIONS_EXT,                VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT                },
  { EPipelineDynamicState::RAY_TRACING_PIPELINE_STACK_SIZE_KHR, VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR },
  { EPipelineDynamicState::VIEWPORT_SHADING_RATE_PALETTE_NV,    VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV    },
  { EPipelineDynamicState::VIEWPORT_COARSE_SAMPLE_ORDER_NV,     VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV     },
  { EPipelineDynamicState::EXCLUSIVE_SCISSOR_NV,                VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV                },
  { EPipelineDynamicState::FRAGMENT_SHADING_RATE_KHR,           VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR           },
  { EPipelineDynamicState::LINE_STIPPLE_EXT,                    VK_DYNAMIC_STATE_LINE_STIPPLE_EXT                    },
  { EPipelineDynamicState::VERTEX_INPUT_EXT,                    VK_DYNAMIC_STATE_VERTEX_INPUT_EXT                    },
  { EPipelineDynamicState::PATCH_CONTROL_POINTS_EXT,            VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT            },
  { EPipelineDynamicState::LOGIC_OP_EXT,                        VK_DYNAMIC_STATE_LOGIC_OP_EXT                        },
  { EPipelineDynamicState::COLOR_WRITE_ENABLE_EXT,              VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT              }
};

// ======= END: Pipeline dynamic state mapping   ==============================

// ======= BEGIN: Texture filter REVERSE mapping & functions ==================

static const std::unordered_map<VkFilter, ETextureFilter> filterReverseMapping = reverseMap(filterMapping);

VkFilter g_getVulkanTextureFilterType(ETextureFilter textureFilter) {
  return getEnumMapping(filterMapping, textureFilter, VK_FILTER_MAX_ENUM);
}

ETextureFilter g_getVulkanTextureFilterType(VkFilter filter) {
  return getEnumMapping(filterReverseMapping, filter, ETextureFilter::MAX);
}

// ======= END: Texture filter REVERSE mapping & functions   ==================

// ======= BEGIN: Texture address mode REVERSE mapping & functions ============

static const std::unordered_map<VkSamplerAddressMode, ETextureAddressMode> addressModeReverseMapping
    = reverseMap(addressModeMapping);

VkSamplerAddressMode g_getVulkanTextureAddressMode(ETextureAddressMode addressMode) {
  return getEnumMapping(addressModeMapping, addressMode, VK_SAMPLER_ADDRESS_MODE_MAX_ENUM);
}

ETextureAddressMode g_getVulkanTextureAddressMode(VkSamplerAddressMode addressMode) {
  return getEnumMapping(addressModeReverseMapping, addressMode, ETextureAddressMode::MAX);
}

// ======= END: Texture address mode REVERSE mapping & functions   ============

// ======= BEGIN: Mipmap mode REVERSE mapping & functions =====================

static const std::unordered_map<VkSamplerMipmapMode, ETextureFilter> mipmapModeReverseMapping
    = reverseMap(mipmapModeMapping);

VkSamplerMipmapMode g_getVulkanTextureMipmapMode(ETextureFilter textureFilter) {
  return getEnumMapping(mipmapModeMapping, textureFilter, VK_SAMPLER_MIPMAP_MODE_MAX_ENUM);
}

ETextureFilter g_getVulkanTextureMipmapMode(VkSamplerMipmapMode mipmapMode) {
  return getEnumMapping(mipmapModeReverseMapping, mipmapMode, ETextureFilter::MAX);
}

// ======= END: Mipmap mode REVERSE mapping & functions   =====================

// ======= BEGIN: Texture format REVERSE mapping & functions ==================

static const std::unordered_map<VkFormat, ETextureFormat> formatReverseMapping = reverseMap(formatMapping);

VkFormat g_getVulkanTextureFormat(ETextureFormat textureFormat) {
  return getEnumMapping(formatMapping, textureFormat, VK_FORMAT_UNDEFINED);
}

ETextureFormat g_getVulkanTextureFormat(VkFormat format) {
  return getEnumMapping(formatReverseMapping, format, ETextureFormat::MAX);
}

// ======= END: Texture format REVERSE mapping & functions   ==================

// ======= BEGIN: Primitive topology REVERSE mapping & functions ==============

static const std::unordered_map<VkPrimitiveTopology, EPrimitiveType> topologyReverseMapping
    = reverseMap(topologyMapping);

VkPrimitiveTopology g_getVulkanPrimitiveTopology(EPrimitiveType primitiveType) {
  return getEnumMapping(topologyMapping, primitiveType, VK_PRIMITIVE_TOPOLOGY_MAX_ENUM);
}

EPrimitiveType g_getVulkanPrimitiveTopology(VkPrimitiveTopology primitiveTopology) {
  return getEnumMapping(topologyReverseMapping, primitiveTopology, EPrimitiveType::MAX);
}

// ======= END: Primitive topology REVERSE mapping & functions   ==============

// ======= BEGIN: Vertex input rate REVERSE mapping & functions ===============

static const std::unordered_map<VkVertexInputRate, EVertexInputRate> inputRateReverseMapping
    = reverseMap(inputRateMapping);

VkVertexInputRate g_getVulkanVertexInputRate(EVertexInputRate inputRate) {
  return getEnumMapping(inputRateMapping, inputRate, VK_VERTEX_INPUT_RATE_MAX_ENUM);
}

EVertexInputRate g_getVulkanVertexInputRate(VkVertexInputRate inputRate) {
  return getEnumMapping(inputRateReverseMapping, inputRate, EVertexInputRate::MAX);
}

// ======= END: Vertex input rate REVERSE mapping & functions   ===============

// ======= BEGIN: Polygon mode REVERSE mapping & functions ====================

static const std::unordered_map<VkPolygonMode, EPolygonMode> polygonModeReverseMapping = reverseMap(polygonModeMapping);

VkPolygonMode g_getVulkanPolygonMode(EPolygonMode polygonMode) {
  return getEnumMapping(polygonModeMapping, polygonMode, VK_POLYGON_MODE_MAX_ENUM);
}

EPolygonMode g_getVulkanPolygonMode(VkPolygonMode polygonMode) {
  return getEnumMapping(polygonModeReverseMapping, polygonMode, EPolygonMode::MAX);
}

// ======= END: Polygon mode REVERSE mapping & functions   ====================

// ======= BEGIN: Front face REVERSE mapping & functions ======================

static const std::unordered_map<VkFrontFace, EFrontFace> frontFaceReverseMapping = reverseMap(frontFaceMapping);

VkFrontFace g_getVulkanFrontFace(EFrontFace frontFace) {
  return getEnumMapping(frontFaceMapping, frontFace, VK_FRONT_FACE_MAX_ENUM);
}

EFrontFace g_getVulkanFrontFace(VkFrontFace frontFace) {
  return getEnumMapping(frontFaceReverseMapping, frontFace, EFrontFace::MAX);
}

// ======= END: Front face REVERSE mapping & functions   ======================

// ======= BEGIN: Stencil op REVERSE mapping & functions ======================

static const std::unordered_map<VkStencilOp, EStencilOp> stencilOpReverseMapping = reverseMap(stencilOpMapping);

VkStencilOp g_getVulkanStencilOp(EStencilOp stencilOp) {
  return getEnumMapping(stencilOpMapping, stencilOp, VK_STENCIL_OP_MAX_ENUM);
}

EStencilOp g_getVulkanStencilOp(VkStencilOp stencilOp) {
  return getEnumMapping(stencilOpReverseMapping, stencilOp, EStencilOp::MAX);
}

// ======= END: Stencil op REVERSE mapping & functions   ======================

// ======= BEGIN: Compare op REVERSE mapping & functions ======================

static const std::unordered_map<VkCompareOp, ECompareOp> compareOpReverseMapping = reverseMap(compareOpMapping);

VkCompareOp g_getVulkanCompareOp(ECompareOp compareOp) {
  return getEnumMapping(compareOpMapping, compareOp, VK_COMPARE_OP_MAX_ENUM);
}

ECompareOp g_getVulkanCompareOp(VkCompareOp compareOp) {
  return getEnumMapping(compareOpReverseMapping, compareOp, ECompareOp::MAX);
}

// ======= END: Compare op REVERSE mapping & functions   ======================

// ======= BEGIN: Blend factor REVERSE mapping & functions ====================

static const std::unordered_map<VkBlendFactor, EBlendFactor> blendFactorReverseMapping = reverseMap(blendFactorMapping);

VkBlendFactor g_getVulkanBlendFactor(EBlendFactor blendFactor) {
  return getEnumMapping(blendFactorMapping, blendFactor, VK_BLEND_FACTOR_MAX_ENUM);
}

EBlendFactor g_getVulkanBlendFactor(VkBlendFactor blendFactor) {
  return getEnumMapping(blendFactorReverseMapping, blendFactor, EBlendFactor::MAX);
}

// ======= END: Blend factor REVERSE mapping & functions   ====================

// ======= BEGIN: Blend op REVERSE mapping & functions ========================

static const std::unordered_map<VkBlendOp, EBlendOp> blendOpReverseMapping = reverseMap(blendOpMapping);

VkBlendOp g_getVulkanBlendOp(EBlendOp blendOp) {
  return getEnumMapping(blendOpMapping, blendOp, VK_BLEND_OP_MAX_ENUM);
}

EBlendOp g_getVulkanBlendOp(VkBlendOp blendOp) {
  return getEnumMapping(blendOpReverseMapping, blendOp, EBlendOp::MAX);
}

// ======= END: Blend op REVERSE mapping & functions   ========================

// ======= BEGIN: Image layout REVERSE mapping & functions ====================

static const std::unordered_map<VkImageLayout, EResourceLayout> imageLayoutReverseMapping
    = reverseMap(imageLayoutMapping);

VkImageLayout g_getVulkanImageLayout(EResourceLayout resourceLayout) {
  return getEnumMapping(imageLayoutMapping, resourceLayout, VK_IMAGE_LAYOUT_MAX_ENUM);
}

EResourceLayout g_getVulkanImageLayout(VkImageLayout resourceLayout) {
  return getEnumMapping(imageLayoutReverseMapping, resourceLayout, EResourceLayout::MAX);
}

// ======= END: Image layout REVERSE mapping & functions   ====================

// ======= BEGIN: Pipeline dynamic state REVERSE mapping & functions ==========

static const std::unordered_map<VkDynamicState, EPipelineDynamicState> dynamicStateReverseMapping
    = reverseMap(dynamicStateMapping);

VkDynamicState g_getVulkanPipelineDynamicState(EPipelineDynamicState dynamicState) {
  return getEnumMapping(dynamicStateMapping, dynamicState, VK_DYNAMIC_STATE_MAX_ENUM);
}

EPipelineDynamicState g_getVulkanPipelineDynamicState(VkDynamicState dynamicState) {
  return getEnumMapping(dynamicStateReverseMapping, dynamicState, EPipelineDynamicState::MAX);
}

// ======= END: Pipeline dynamic state REVERSE mapping & functions   ==========

// clang-format on

}  // namespace game_engine
