#ifndef GAME_ENGINE_UTILS_DX12_H
#define GAME_ENGINE_UTILS_DX12_H

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "platform/windows/windows_platform_setup.h"
#include "utils/memory/align.h"

#include <cassert>
#include <cstdint>
#include <memory>

namespace game_engine {

struct jBuffer_DX12;
struct jTexture_DX12;
struct ImageSubResourceData;

// TODO: why need anonymous namespace in header?
// namespace {
size_t BitsPerPixel_(DXGI_FORMAT fmt);

size_t BytesPerPixel_(DXGI_FORMAT fmt);

const D3D12_HEAP_PROPERTIES& GetUploadHeap();

const D3D12_HEAP_PROPERTIES& GetDefaultHeap();

const D3D12_RESOURCE_DESC& GetUploadResourceDesc(uint64_t InSize);

ComPtr<ID3D12Resource> CreateStagingBuffer(const void* InInitData,
                                           int64_t     InSize,
                                           uint64_t    InAlignment = 1);

void UploadByUsingStagingBuffer(ComPtr<ID3D12Resource>& DestBuffer,
                                const void*             InInitData,
                                uint64_t                InSize,
                                uint64_t                InAlignment = 1);

D3D12_RESOURCE_DESC GetDefaultResourceDesc(uint64_t InAlignedSize,
                                           bool     InIsAllowUAV);

ComPtr<ID3D12Resource> CreateDefaultResource(
    uint64_t              InAlignedSize,
    D3D12_RESOURCE_STATES InInitialState,
    bool                  InIsAllowUAV,
    bool                  InIsCPUAccessible,
    const wchar_t*        InName = nullptr);

void* CopyInitialData(ComPtr<ID3D12Resource>& InDest,
                      const void*             InInitData,
                      uint64_t                InSize,
                      uint64_t                InAlignment,
                      bool                    InIsCPUAccessible);

//}  // namespace

std::shared_ptr<jCreatedResource> CreateBufferInternal(
    uint64_t              InSize,
    uint64_t              InAlignment,
    EBufferCreateFlag     InBufferCreateFlag,
    D3D12_RESOURCE_STATES InInitialResourceState,
    const wchar_t*        InResourceName = nullptr);

std::shared_ptr<jBuffer_DX12> CreateBuffer(uint64_t          InSize,
                                           uint64_t          InAlignment,
                                           EBufferCreateFlag InBufferCreateFlag,
                                           EResourceLayout   InLayout,
                                           const void*       InData = nullptr,
                                           uint64_t          InDataSize = 0,
                                           const wchar_t*    InResourceName
                                           = nullptr);

std::shared_ptr<jCreatedResource> CreateTexturenternal(
    uint32_t                 InWidth,
    uint32_t                 InHeight,
    uint32_t                 InArrayLayers,
    uint32_t                 InMipLevels,
    uint32_t                 InNumOfSample,
    D3D12_RESOURCE_DIMENSION InType,
    DXGI_FORMAT              InFormat,
    ETextureCreateFlag       InTextureCreateFlag,
    EResourceLayout          InImageLayout  = EResourceLayout::UNDEFINED,
    D3D12_CLEAR_VALUE*       InClearValue   = nullptr,
    const wchar_t*           InResourceName = nullptr);

std::shared_ptr<jTexture_DX12> CreateTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InArrayLayers,
    uint32_t             InMipLevels,
    uint32_t             InNumOfSample,
    ETextureType         InType,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout  = EResourceLayout::UNDEFINED,
    const jRTClearValue& InClearValue   = jRTClearValue::Invalid,
    const wchar_t*       InResourceName = nullptr);

std::shared_ptr<jTexture_DX12> CreateTexture(
    const std::shared_ptr<jCreatedResource>& InTexture,
    ETextureCreateFlag                       InTextureCreateFlag,
    EResourceLayout                          InImageLayout,
    const jRTClearValue&                     InClearValue,
    const wchar_t*                           InResourceName);

uint64_t CopyBufferToTexture(ID3D12GraphicsCommandList4* InCommandBuffer,
                             ID3D12Resource*             InBuffer,
                             uint64_t                    InBufferOffset,
                             ID3D12Resource*             InImage,
                             int32_t InImageSubresourceIndex = 0);
uint64_t CopyBufferToTexture(ID3D12GraphicsCommandList4* InCommandBuffer,
                             ID3D12Resource*             InBuffer,
                             uint64_t                    InBufferOffset,
                             ID3D12Resource*             InImage,
                             int32_t InNumOfImageSubresource,
                             int32_t InStartImageSubresource);
void     CopyBufferToTexture(
        ID3D12GraphicsCommandList4*              InCommandBuffer,
        ID3D12Resource*                          InBuffer,
        ID3D12Resource*                          InImage,
        const std::vector<ImageSubResourceData>& InSubresourceData);
void CopyBuffer(ID3D12GraphicsCommandList4* InCommandBuffer,
                ID3D12Resource*             InSrcBuffer,
                ID3D12Resource*             InDstBuffer,
                uint64_t                    InSize,
                uint64_t                    InSrcOffset,
                uint64_t                    InDstOffset);
void CopyBuffer(ID3D12Resource* InSrcBuffer,
                ID3D12Resource* InDstBuffer,
                uint64_t        InSize,
                uint64_t        InSrcOffset,
                uint64_t        InDstOffset);

// Create CBV
void CreateConstantBufferView(jBuffer_DX12* InBuffer);

// Create SRV for Buffer
void CreateShaderResourceView_StructuredBuffer(jBuffer_DX12* InBuffer,
                                               uint32_t      InStride,
                                               uint32_t      InCount);
void CreateShaderResourceView_Raw(jBuffer_DX12* InBuffer,
                                  uint32_t      InBufferSize);
void CreateShaderResourceView_Formatted(jBuffer_DX12*  InBuffer,
                                        ETextureFormat InFormat,
                                        uint32_t       InBufferSize);

// Create UAV for Buffer
void CreateUnorderedAccessView_StructuredBuffer(jBuffer_DX12* InBuffer,
                                                uint32_t      InStride,
                                                uint32_t      InCount);
void CreateUnorderedAccessView_Raw(jBuffer_DX12* InBuffer,
                                   uint32_t      InBufferSize);
void CreateUnorderedAccessView_Formatted(jBuffer_DX12*  InBuffer,
                                         ETextureFormat InFormat,
                                         uint32_t       InBufferSize);

// Create SRV for Texture
void CreateShaderResourceView(jTexture_DX12* InTexture);

// Create UAV for Texture
void CreateUnorderedAccessView(jTexture_DX12* InTexture);

void CreateDepthStencilView(jTexture_DX12* InTexture);
void CreateRenderTargetView(jTexture_DX12* InTexture);

}  // namespace game_engine

#endif  // GAME_ENGINE_UTILS_DX12_H