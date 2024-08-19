#ifndef GAME_ENGINE_UTILS_DX12_H
#define GAME_ENGINE_UTILS_DX12_H

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "platform/windows/windows_platform_setup.h"
#include "utils/memory/align.h"

#include <cassert>
#include <cstdint>
#include <memory>

namespace game_engine {

struct BufferDx12;
struct TextureDx12;
struct ImageSubResourceData;

// TODO: why need anonymous namespace in header?
// namespace {
size_t BitsPerPixel_(DXGI_FORMAT fmt);

size_t BytesPerPixel_(DXGI_FORMAT fmt);

const D3D12_HEAP_PROPERTIES& GetUploadHeap();

const D3D12_HEAP_PROPERTIES& GetDefaultHeap();

const D3D12_RESOURCE_DESC& GetUploadResourceDesc(uint64_t size);

ComPtr<ID3D12Resource> CreateStagingBuffer(const void* initData,
                                           int64_t     size,
                                           uint64_t    alignment = 1);

void UploadByUsingStagingBuffer(ComPtr<ID3D12Resource>& DestBuffer,
                                const void*             initData,
                                uint64_t                size,
                                uint64_t                alignment = 1);

D3D12_RESOURCE_DESC GetDefaultResourceDesc(uint64_t alignedSize,
                                           bool     isAllowUAV);

ComPtr<ID3D12Resource> CreateDefaultResource(
    uint64_t              alignedSize,
    D3D12_RESOURCE_STATES initialState,
    bool                  isAllowUAV,
    bool                  isCPUAccessible,
    const wchar_t*        name = nullptr);

void* CopyInitialData(ComPtr<ID3D12Resource>& dest,
                      const void*             initData,
                      uint64_t                size,
                      uint64_t                alignment,
                      bool                    isCPUAccessible);

//}  // namespace

std::shared_ptr<CreatedResource> CreateBufferInternal(
    uint64_t              size,
    uint64_t              alignment,
    EBufferCreateFlag     bufferCreateFlag,
    D3D12_RESOURCE_STATES initialResourceState,
    const wchar_t*        resourceName = nullptr);

std::shared_ptr<BufferDx12> CreateBuffer(uint64_t          size,
                                           uint64_t          alignment,
                                           EBufferCreateFlag bufferCreateFlag,
                                           EResourceLayout   layout,
                                           const void*       data = nullptr,
                                           uint64_t          dataSize = 0,
                                           const wchar_t*    resourceName
                                           = nullptr);

std::shared_ptr<CreatedResource> CreateTexturenternal(
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

std::shared_ptr<TextureDx12> CreateTexture(
    uint32_t             witdh,
    uint32_t             height,
    uint32_t             arrayLayers,
    uint32_t             mipLevels,
    uint32_t             numOfSamples,
    ETextureType         type,
    ETextureFormat       format,
    ETextureCreateFlag   textureCreateFlag,
    EResourceLayout      imageLayout  = EResourceLayout::UNDEFINED,
    const RTClearValue& clearValue   = RTClearValue::s_kInvalid,
    const wchar_t*       resourceName = nullptr);

std::shared_ptr<TextureDx12> CreateTexture(
    const std::shared_ptr<CreatedResource>& texture,
    ETextureCreateFlag                       textureCreateFlag,
    EResourceLayout                          imageLayout,
    const RTClearValue&                     clearValue,
    const wchar_t*                           resourceName);

uint64_t CopyBufferToTexture(ID3D12GraphicsCommandList4* commandBuffer,
                             ID3D12Resource*             buffer,
                             uint64_t                    bufferOffset,
                             ID3D12Resource*             image,
                             int32_t imageSubresourceIndex = 0);
uint64_t CopyBufferToTexture(ID3D12GraphicsCommandList4* commandBuffer,
                             ID3D12Resource*             buffer,
                             uint64_t                    bufferOffset,
                             ID3D12Resource*             image,
                             int32_t numOfImageSubresources,
                             int32_t startImageSubresources);
void     CopyBufferToTexture(
        ID3D12GraphicsCommandList4*              commandBuffer,
        ID3D12Resource*                          buffer,
        ID3D12Resource*                          image,
        const std::vector<ImageSubResourceData>& subresourceData);
void CopyBuffer(ID3D12GraphicsCommandList4* commandBuffer,
                ID3D12Resource*             srcBuffer,
                ID3D12Resource*             dstBuffer,
                uint64_t                    size,
                uint64_t                    srcOffset,
                uint64_t                    dstOffset);
void CopyBuffer(ID3D12Resource* srcBuffer,
                ID3D12Resource* dstBuffer,
                uint64_t        size,
                uint64_t        srcOffset,
                uint64_t        dstOffset);

// Create CBV
void CreateConstantBufferView(BufferDx12* buffer);

// Create SRV for m_buffer
void CreateShaderResourceView_StructuredBuffer(BufferDx12* buffer,
                                               uint32_t      stride,
                                               uint32_t      count);
void CreateShaderResourceView_Raw(BufferDx12* buffer,
                                  uint32_t      bufferSize);
void CreateShaderResourceView_Formatted(BufferDx12*  buffer,
                                        ETextureFormat format,
                                        uint32_t       bufferSize);

// Create UAV for m_buffer
void CreateUnorderedAccessView_StructuredBuffer(BufferDx12* buffer,
                                                uint32_t      stride,
                                                uint32_t      count);
void CreateUnorderedAccessView_Raw(BufferDx12* buffer,
                                   uint32_t      bufferSize);
void CreateUnorderedAccessView_Formatted(BufferDx12*  buffer,
                                         ETextureFormat format,
                                         uint32_t       bufferSize);

// Create SRV for Texture
void CreateShaderResourceView(TextureDx12* texture);

// Create UAV for Texture
void CreateUnorderedAccessView(TextureDx12* texture);

void CreateDepthStencilView(TextureDx12* texture);
void CreateRenderTargetView(TextureDx12* texture);

}  // namespace game_engine

#endif  // GAME_ENGINE_UTILS_DX12_H