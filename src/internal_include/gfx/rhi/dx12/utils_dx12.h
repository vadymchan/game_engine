#ifndef GAME_ENGINE_UTILS_DX12_H
#define GAME_ENGINE_UTILS_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "utils/memory/align.h"

#include <cassert>
#include <cstdint>
#include <memory>

namespace game_engine {

struct BufferDx12;
struct TextureDx12;

struct Image;

// TODO: why need anonymous namespace in header?
// TODO: check whether these functions are used
// namespace {
size_t g_bitsPerPixel(DXGI_FORMAT fmt);

size_t g_bytesPerPixel(DXGI_FORMAT fmt);

const D3D12_HEAP_PROPERTIES& g_getUploadHeap();

const D3D12_HEAP_PROPERTIES& g_getDefaultHeap();

const D3D12_RESOURCE_DESC& g_getUploadResourceDesc(uint64_t size);

ComPtr<ID3D12Resource> g_createStagingBuffer(const void* initData,
                                             int64_t     size,
                                             uint64_t    alignment = 1);

void g_uploadByUsingStagingBuffer(ComPtr<ID3D12Resource>& DestBuffer,
                                  const void*             initData,
                                  uint64_t                size,
                                  uint64_t                alignment = 1);

D3D12_RESOURCE_DESC g_getDefaultResourceDesc(uint64_t alignedSize,
                                             bool     isAllowUAV);

ComPtr<ID3D12Resource> g_createDefaultResource(
    uint64_t              alignedSize,
    D3D12_RESOURCE_STATES initialState,
    bool                  isAllowUAV,
    bool                  isCPUAccessible,
    const wchar_t*        name = nullptr);

void* g_copyInitialData(ComPtr<ID3D12Resource>& dest,
                        const void*             initData,
                        uint64_t                size,
                        uint64_t                alignment,
                        bool                    isCPUAccessible);

//}  // namespace

std::shared_ptr<CreatedResourceDx12> g_createBufferInternal(
    uint64_t              size,
    uint64_t              alignment,
    EBufferCreateFlag     bufferCreateFlag,
    D3D12_RESOURCE_STATES initialResourceState,
    const wchar_t*        resourceName = nullptr);

std::shared_ptr<BufferDx12> g_createBuffer(uint64_t          size,
                                           uint64_t          alignment,
                                           EBufferCreateFlag bufferCreateFlag,
                                           EResourceLayout   layout,
                                           const void*       data     = nullptr,
                                           uint64_t          dataSize = 0,
                                           const wchar_t*    resourceName
                                           = nullptr);

std::shared_ptr<CreatedResourceDx12> g_createTexturenternal(
    uint32_t                 witdh,
    uint32_t                 height,
    uint32_t                 arrayLayers,
    uint32_t                 mipLevels,
    uint32_t                 numOfSamples,
    D3D12_RESOURCE_DIMENSION type,
    DXGI_FORMAT              format,
    ETextureCreateFlag       textureCreateFlag,
    EResourceLayout          imageLayout  = EResourceLayout::UNDEFINED,
    D3D12_CLEAR_VALUE*       clearValue   = nullptr,
    const wchar_t*           resourceName = nullptr);

std::shared_ptr<TextureDx12> g_createTexture(
    uint32_t            witdh,
    uint32_t            height,
    uint32_t            arrayLayers,
    uint32_t            mipLevels,
    uint32_t            numOfSamples,
    ETextureType        type,
    ETextureFormat      format,
    ETextureCreateFlag  textureCreateFlag,
    EResourceLayout     imageLayout  = EResourceLayout::UNDEFINED,
    const RtClearValue& clearValue   = RtClearValue::s_kInvalid,
    const wchar_t*      resourceName = nullptr);

std::shared_ptr<TextureDx12> g_createTexture(
    const std::shared_ptr<CreatedResourceDx12>& texture,
    ETextureCreateFlag                          textureCreateFlag,
    EResourceLayout                             imageLayout,
    const RtClearValue&                         clearValue,
    const wchar_t*                              resourceName);

void g_copyBufferToTexture(ID3D12GraphicsCommandList4*   commandBuffer,
                           ID3D12Resource*               buffer,
                           ID3D12Resource*               imageResource,
                           const std::shared_ptr<Image>& image);
void g_copyBuffer(ID3D12GraphicsCommandList4* commandBuffer,
                  ID3D12Resource*             srcBuffer,
                  ID3D12Resource*             dstBuffer,
                  uint64_t                    size,
                  uint64_t                    srcOffset,
                  uint64_t                    dstOffset);
void g_copyBuffer(ID3D12Resource* srcBuffer,
                  ID3D12Resource* dstBuffer,
                  uint64_t        size,
                  uint64_t        srcOffset,
                  uint64_t        dstOffset);

void g_copyTexture(ID3D12GraphicsCommandList* commandList,
                   ID3D12Resource*            srcResource,
                   D3D12_RESOURCE_STATES      srcState,
                   ID3D12Resource*            dstResource,
                   D3D12_RESOURCE_STATES      dstState,
                   uint32_t                   srcWidth,
                   uint32_t                   srcHeight,
                   uint32_t                   dstWidth,
                   uint32_t                   dstHeight,
                   uint32_t                   srcArrayLayers = 1,
                   uint32_t                   dstArrayLayers = 1);

void g_copyTexture(ID3D12GraphicsCommandList*          commandList,
                   const std::shared_ptr<TextureDx12>& srcTexture,
                   const std::shared_ptr<TextureDx12>& dstTexture);

// Create CBV
void g_createConstantBufferView(BufferDx12* buffer);

// Create SRV for m_buffer
void g_createShaderResourceViewStructuredBuffer(BufferDx12* buffer,
                                                uint32_t    stride,
                                                uint32_t    count);
void g_createShaderResourceViewRaw(BufferDx12* buffer, uint32_t bufferSize);
void g_createShaderResourceViewFormatted(BufferDx12*    buffer,
                                         ETextureFormat format,
                                         uint32_t       bufferSize);

// Create UAV for m_buffer
void g_createUnorderedAccessViewStructuredBuffer(BufferDx12* buffer,
                                                 uint32_t    stride,
                                                 uint32_t    count);
void g_createUnorderedAccessViewRaw(BufferDx12* buffer, uint32_t bufferSize);
void g_createUnorderedAccessViewFormatted(BufferDx12*    buffer,
                                          ETextureFormat format,
                                          uint32_t       bufferSize);

// Create SRV for Texture
void g_createShaderResourceView(const std::shared_ptr<TextureDx12>& texture);

// Create UAV for Texture
void g_createUnorderedAccessView(const std::shared_ptr<TextureDx12>& texture);

void g_createDepthStencilView(const std::shared_ptr<TextureDx12>& texture);
void g_createRenderTargetView(const std::shared_ptr<TextureDx12>& texture);

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_UTILS_DX12_H
