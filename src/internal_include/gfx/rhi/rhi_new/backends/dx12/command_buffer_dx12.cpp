#include "gfx/rhi/rhi_new/backends/dx12/command_buffer_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/rhi_new/backends/dx12/buffer_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/descriptor_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/device_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/framebuffer_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/pipeline_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/render_pass_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/rhi_enums_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/texture_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

//-------------------------------------------------------------------------
// CommandBufferDx12 implementation
//-------------------------------------------------------------------------

CommandBufferDx12::CommandBufferDx12(DeviceDx12*                device,
                                     ID3D12GraphicsCommandList* commandList,
                                     ID3D12CommandAllocator*    commandAllocator)
    : m_device_(device)
    , m_commandList_(commandList)
    , m_commandAllocator_(commandAllocator) {
}

void CommandBufferDx12::begin() {
  if (m_isRecording_) {
    GlobalLogger::Log(LogLevel::Warning, "CommandBufferDx12::begin - Command buffer is already recording");
    return;
  }

  m_isRecording_ = true;
}

void CommandBufferDx12::end() {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Warning, "CommandBufferDx12::end - Command buffer is not recording");
    return;
  }

  // End any active render pass
  if (m_isRenderPassActive_) {
    endRenderPass();
  }

  // Close the command list
  HRESULT hr = m_commandList_->Close();
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::end - Failed to close command list");
    return;
  }

  m_isRecording_ = false;
}

void CommandBufferDx12::reset() {
  if (m_isRecording_) {
    GlobalLogger::Log(LogLevel::Warning, "CommandBufferDx12::reset - Cannot reset while recording");
    return;
  }

  // Reset command allocator
  HRESULT hr = m_commandAllocator_->Reset();
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::reset - Failed to reset command allocator");
    return;
  }

  // Reset command list
  hr = m_commandList_->Reset(m_commandAllocator_, nullptr);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::reset - Failed to reset command list");
    return;
  }

  m_currentPipeline_    = nullptr;
  m_currentRenderPass_  = nullptr;
  m_currentFramebuffer_ = nullptr;
  m_isRenderPassActive_ = false;
}

void CommandBufferDx12::setPipeline(Pipeline* pipeline) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::setPipeline - Command buffer is not recording");
    return;
  }

  auto pipelineDx12 = dynamic_cast<GraphicsPipelineDx12*>(pipeline);
  if (!pipelineDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::setPipeline - Invalid pipeline type");
    return;
  }

  // Set PSO (Pipeline State Object)
  m_commandList_->SetPipelineState(pipelineDx12->getPipelineState());

  // Set root signature (graphics / compute)
  auto pipelineType = pipelineDx12->getType();
  if (pipelineType == PipelineType::Graphics) {
    m_commandList_->SetGraphicsRootSignature(pipelineDx12->getRootSignature());

    // Set blend factors - in DirectX 12, blend constants are not part of the PSO
    // but are set dynamically when binding the pipeline
    m_commandList_->OMSetBlendFactor(pipelineDx12->getBlendFactors().data());

    // Set primitive topology
    D3D_PRIMITIVE_TOPOLOGY topology = g_getPrimitiveTopologyDx12(pipelineDx12->getPrimitiveType());
    m_commandList_->IASetPrimitiveTopology(topology);
  } else if (pipelineType == PipelineType::Compute) {
    m_commandList_->SetComputeRootSignature(pipelineDx12->getRootSignature());
  }

  m_currentPipeline_ = pipelineDx12;
}

void CommandBufferDx12::setViewport(const Viewport& viewport) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::setViewport - Command buffer is not recording");
    return;
  }

  D3D12_VIEWPORT viewportDx12;
  viewportDx12.TopLeftX = viewport.x;
  viewportDx12.TopLeftY = viewport.y;
  viewportDx12.Width    = viewport.width;
  viewportDx12.Height   = viewport.height;
  viewportDx12.MinDepth = viewport.minDepth;
  viewportDx12.MaxDepth = viewport.maxDepth;

  m_commandList_->RSSetViewports(1, &viewportDx12);
}

void CommandBufferDx12::setScissor(const ScissorRect& scissor) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::setScissor - Command buffer is not recording");
    return;
  }

  D3D12_RECT scissorDx12;
  scissorDx12.left   = scissor.x;
  scissorDx12.top    = scissor.y;
  scissorDx12.right  = scissor.x + scissor.width;
  scissorDx12.bottom = scissor.y + scissor.height;

  m_commandList_->RSSetScissorRects(1, &scissorDx12);
}

void CommandBufferDx12::bindVertexBuffer(uint32_t binding, Buffer* buffer, uint64_t offset) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindVertexBuffer - Command buffer is not recording");
    return;
  }

  BufferDx12* bufferDx12 = dynamic_cast<BufferDx12*>(buffer);
  if (!bufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindVertexBuffer - Invalid buffer type");
    return;
  }

  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
  vertexBufferView.BufferLocation = bufferDx12->getGPUVirtualAddress() + offset;
  vertexBufferView.SizeInBytes    = static_cast<UINT>(bufferDx12->getSize() - offset);
  vertexBufferView.StrideInBytes  = bufferDx12->getStride();

  m_commandList_->IASetVertexBuffers(binding, 1, &vertexBufferView);
}

void CommandBufferDx12::bindIndexBuffer(Buffer* buffer, uint64_t offset, bool use32BitIndices) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindIndexBuffer - Command buffer is not recording");
    return;
  }

  BufferDx12* bufferDx12 = dynamic_cast<BufferDx12*>(buffer);
  if (!bufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindIndexBuffer - Invalid buffer type");
    return;
  }

  D3D12_INDEX_BUFFER_VIEW indexBufferView;
  indexBufferView.BufferLocation = bufferDx12->getGPUVirtualAddress() + offset;
  indexBufferView.SizeInBytes    = static_cast<UINT>(bufferDx12->getSize() - offset);
  indexBufferView.Format         = use32BitIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

  m_commandList_->IASetIndexBuffer(&indexBufferView);
}

void CommandBufferDx12::bindDescriptorSet(uint32_t setIndex, DescriptorSet* set) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindDescriptorSet - Command buffer is not recording");
    return;
  }

  if (!m_currentPipeline_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindDescriptorSet - No active pipeline");
    return;
  }

  DescriptorSetDx12* descriptorSetDx12 = dynamic_cast<DescriptorSetDx12*>(set);
  if (!descriptorSetDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::bindDescriptorSet - Invalid descriptor set type");
    return;
  }

  // In DX12, we bind descriptor tables to root parameter slots
  switch (m_currentPipeline_->getType()) {
    case PipelineType::Graphics:
      m_commandList_->SetGraphicsRootDescriptorTable(setIndex, descriptorSetDx12->getGPUHandle());
      break;
    case PipelineType::Compute:
      m_commandList_->SetComputeRootDescriptorTable(setIndex, descriptorSetDx12->getGPUHandle());
      break;
  }
}

void CommandBufferDx12::draw(uint32_t vertexCount, uint32_t firstVertex) {
  if (!m_isRecording_ || !m_isRenderPassActive_) {
    GlobalLogger::Log(LogLevel::Error,
                      "CommandBufferDx12::draw - Command buffer is not recording or render pass is not active");
    return;
  }

  m_commandList_->DrawInstanced(vertexCount, 1, firstVertex, 0);
}

void CommandBufferDx12::drawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset) {
  if (!m_isRecording_ || !m_isRenderPassActive_) {
    GlobalLogger::Log(LogLevel::Error,
                      "CommandBufferDx12::drawIndexed - Command buffer is not recording or render pass is not active");
    return;
  }

  m_commandList_->DrawIndexedInstanced(indexCount, 1, firstIndex, vertexOffset, 0);
}

void CommandBufferDx12::drawInstanced(uint32_t vertexCount,
                                      uint32_t instanceCount,
                                      uint32_t firstVertex,
                                      uint32_t firstInstance) {
  if (!m_isRecording_ || !m_isRenderPassActive_) {
    GlobalLogger::Log(
        LogLevel::Error,
        "CommandBufferDx12::drawInstanced - Command buffer is not recording or render pass is not active");
    return;
  }

  m_commandList_->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBufferDx12::drawIndexedInstanced(
    uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
  if (!m_isRecording_ || !m_isRenderPassActive_) {
    GlobalLogger::Log(
        LogLevel::Error,
        "CommandBufferDx12::drawIndexedInstanced - Command buffer is not recording or render pass is not active");
    return;
  }

  m_commandList_->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBufferDx12::resourceBarrier(const ResourceBarrierDesc& barrier) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::resourceBarrier - Command buffer is not recording");
    return;
  }

  if (!barrier.texture) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::resourceBarrier - Null texture");
    return;
  }

  TextureDx12* textureDx12 = dynamic_cast<TextureDx12*>(barrier.texture);
  if (!textureDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::resourceBarrier - Invalid texture type");
    return;
  }

  auto oldState = g_getResourceLayoutDx12(barrier.oldLayout);
  auto newState = g_getResourceLayoutDx12(barrier.newLayout);

  D3D12_RESOURCE_BARRIER barrierDx12 = {};
  barrierDx12.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrierDx12.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrierDx12.Transition.pResource   = textureDx12->getResource();
  barrierDx12.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrierDx12.Transition.StateBefore = g_getResourceLayoutDx12(barrier.oldLayout);
  barrierDx12.Transition.StateAfter  = g_getResourceLayoutDx12(barrier.newLayout);

  m_commandList_->ResourceBarrier(1, &barrierDx12);

  // Update the texture's tracked state AFTER successful barrier command
  textureDx12->updateCurrentState(newState);
}

void CommandBufferDx12::beginRenderPass(RenderPass*                    renderPass,
                                        Framebuffer*                   framebuffer,
                                        const std::vector<ClearValue>& clearValues) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::beginRenderPass - Command buffer is not recording");
    return;
  }

  if (m_isRenderPassActive_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::beginRenderPass - Render pass is already active");
    return;
  }

  RenderPassDx12*  renderPassDx12  = dynamic_cast<RenderPassDx12*>(renderPass);
  FramebufferDx12* framebufferDx12 = dynamic_cast<FramebufferDx12*>(framebuffer);

  if (!renderPassDx12 || !framebufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::beginRenderPass - Invalid render pass or framebuffer type");
    return;
  }

  // Transition render targets to render target state
  framebufferDx12->transitionToRenderTargetState(this);

  // Setup render targets
  uint32_t numRtvs = framebufferDx12->getColorAttachmentCount();
  if (numRtvs > 0) {
    m_commandList_->OMSetRenderTargets(numRtvs,
                                       framebufferDx12->getRTVHandles(),
                                       FALSE,
                                       framebufferDx12->hasDSV() ? framebufferDx12->getDSVHandle() : nullptr);

    // Clear RTVs if requested
    for (uint32_t i = 0; i < numRtvs && i < clearValues.size(); i++) {
      if (renderPassDx12->shouldClearColor(i)) {
        m_commandList_->ClearRenderTargetView(framebufferDx12->getRTVHandles()[i], clearValues[i].color, 0, nullptr);
      }
    }
  }

  // Clear depth/stencil if needed
  if (framebufferDx12->hasDSV() && renderPassDx12->shouldClearDepthStencil()) {
    D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
    if (renderPassDx12->shouldClearStencil()) {
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
    }

    float   depth   = 1.0f;
    uint8_t stencil = 0;

    // Use clear values if provided
    if (!clearValues.empty()) {
      // The depth/stencil clear value is usually the last one
      size_t dsIndex = numRtvs < clearValues.size() ? numRtvs : 0;
      if (dsIndex < clearValues.size()) {
        depth   = clearValues[dsIndex].depthStencil.depth;
        stencil = static_cast<uint8_t>(clearValues[dsIndex].depthStencil.stencil);
      }
    }

    m_commandList_->ClearDepthStencilView(*framebufferDx12->getDSVHandle(), clearFlags, depth, stencil, 0, nullptr);
  }

  m_currentRenderPass_  = renderPassDx12;
  m_currentFramebuffer_ = framebufferDx12;
  m_isRenderPassActive_ = true;
}

void CommandBufferDx12::endRenderPass() {
  if (!m_isRecording_ || !m_isRenderPassActive_) {
    GlobalLogger::Log(
        LogLevel::Error,
        "CommandBufferDx12::endRenderPass - Command buffer is not recording or render pass is not active");
    return;
  }

  // Transition render targets to resource state for potential future use
  if (m_currentFramebuffer_) {
    m_currentFramebuffer_->transitionToResourceState(this);
  }

  m_isRenderPassActive_ = false;
  m_currentRenderPass_  = nullptr;
  m_currentFramebuffer_ = nullptr;
}

void CommandBufferDx12::copyBuffer(
    Buffer* srcBuffer, Buffer* dstBuffer, uint64_t srcOffset, uint64_t dstOffset, uint64_t size) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::copyBuffer - Command buffer is not recording");
    return;
  }

  BufferDx12* srcBufferDx12 = dynamic_cast<BufferDx12*>(srcBuffer);
  BufferDx12* dstBufferDx12 = dynamic_cast<BufferDx12*>(dstBuffer);

  if (!srcBufferDx12 || !dstBufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::copyBuffer - Invalid buffer type");
    return;
  }

  // Determine copy size (if not specified, copy from offset to end of source buffer)
  uint64_t copySize = size;
  if (copySize == 0) {
    copySize = srcBufferDx12->getSize() - srcOffset;
  }

  // Verify copy size doesn't exceed destination buffer
  if (dstOffset + copySize > dstBufferDx12->getSize()) {
    GlobalLogger::Log(LogLevel::Error,
                      "CommandBufferDx12::copyBuffer - Copy operation exceeds destination buffer size");
    return;
  }

  m_commandList_->CopyBufferRegion(
      dstBufferDx12->getResource(), dstOffset, srcBufferDx12->getResource(), srcOffset, copySize);
}

void CommandBufferDx12::copyBufferToTexture(Buffer*  srcBuffer,
                                            Texture* dstTexture,
                                            uint32_t mipLevel,
                                            uint32_t arrayLayer) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::copyBufferToTexture - Command buffer is not recording");
    return;
  }

  BufferDx12*  srcBufferDx12  = dynamic_cast<BufferDx12*>(srcBuffer);
  TextureDx12* dstTextureDx12 = dynamic_cast<TextureDx12*>(dstTexture);

  if (!srcBufferDx12 || !dstTextureDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::copyBufferToTexture - Invalid buffer or texture type");
    return;
  }

  // First, transition texture to copy destination state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = dstTextureDx12->getResource();
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = dstTextureDx12->getCurrentState();
  barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;

  m_commandList_->ResourceBarrier(1, &barrier);

  // Calculate subresource index
  UINT subresource
      = D3D12CalcSubresource(mipLevel, arrayLayer, 0, dstTextureDx12->getMipLevels(), dstTextureDx12->getArraySize());

  // Setup destination subresource
  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource                   = dstTextureDx12->getResource();
  dstLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dstLocation.SubresourceIndex            = subresource;

  // Setup source buffer
  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource                   = srcBufferDx12->getResource();
  srcLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

  // Get footprint information
  D3D12_RESOURCE_DESC textureDesc = dstTextureDx12->getResource()->GetDesc();

  UINT   numRows;
  UINT64 rowSizeInBytes, totalBytes;

  m_device_->getDevice()->GetCopyableFootprints(&textureDesc,
                                                subresource,
                                                1,
                                                0,  // Buffer offset
                                                &srcLocation.PlacedFootprint,
                                                &numRows,
                                                &rowSizeInBytes,
                                                &totalBytes);

  // Copy buffer to texture
  m_commandList_->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

  // Transition texture back to its original state
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter  = dstTextureDx12->getCurrentState();
  m_commandList_->ResourceBarrier(1, &barrier);
}

void CommandBufferDx12::copyTextureToBuffer(Texture* srcTexture,
                                            Buffer*  dstBuffer,
                                            uint32_t mipLevel,
                                            uint32_t arrayLayer) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::copyTextureToBuffer - Command buffer is not recording");
    return;
  }

  TextureDx12* srcTextureDx12 = dynamic_cast<TextureDx12*>(srcTexture);
  BufferDx12*  dstBufferDx12  = dynamic_cast<BufferDx12*>(dstBuffer);

  if (!srcTextureDx12 || !dstBufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::copyTextureToBuffer - Invalid texture or buffer type");
    return;
  }

  // First, transition texture to copy source state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = srcTextureDx12->getResource();
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = srcTextureDx12->getCurrentState();
  barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;

  m_commandList_->ResourceBarrier(1, &barrier);

  // Calculate subresource index
  UINT subresource
      = D3D12CalcSubresource(mipLevel, arrayLayer, 0, srcTextureDx12->getMipLevels(), srcTextureDx12->getArraySize());

  // Setup source texture
  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource                   = srcTextureDx12->getResource();
  srcLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  srcLocation.SubresourceIndex            = subresource;

  // Setup destination buffer
  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource                   = dstBufferDx12->getResource();
  dstLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

  // Get footprint information
  D3D12_RESOURCE_DESC textureDesc = srcTextureDx12->getResource()->GetDesc();

  UINT   numRows;
  UINT64 rowSizeInBytes, totalBytes;

  m_device_->getDevice()->GetCopyableFootprints(&textureDesc,
                                                subresource,
                                                1,
                                                0,  // Buffer offset
                                                &dstLocation.PlacedFootprint,
                                                &numRows,
                                                &rowSizeInBytes,
                                                &totalBytes);

  // Copy texture to buffer
  m_commandList_->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

  // Transition texture back to its original state
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
  barrier.Transition.StateAfter  = srcTextureDx12->getCurrentState();
  m_commandList_->ResourceBarrier(1, &barrier);
}

void CommandBufferDx12::clearColor(Texture* texture, const float color[4], uint32_t mipLevel, uint32_t arrayLayer) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::clearColor - Command buffer is not recording");
    return;
  }

  TextureDx12* textureDx12 = dynamic_cast<TextureDx12*>(texture);
  if (!textureDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::clearColor - Invalid texture type");
    return;
  }

  // Get RTV handle for this mip/array slice
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = textureDx12->getRTVHandle(mipLevel, arrayLayer);
  if (rtvHandle.ptr == 0) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::clearColor - No valid RTV handle for texture");
    return;
  }

  // Transition texture to render target state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = textureDx12->getResource();
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = textureDx12->getCurrentState();
  barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;

  m_commandList_->ResourceBarrier(1, &barrier);

  // Clear the texture
  m_commandList_->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

  // Transition texture back to its original state
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter  = textureDx12->getCurrentState();
  m_commandList_->ResourceBarrier(1, &barrier);
}

void CommandBufferDx12::clearDepthStencil(
    Texture* texture, float depth, uint8_t stencil, uint32_t mipLevel, uint32_t arrayLayer) {
  if (!m_isRecording_) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::clearDepthStencil - Command buffer is not recording");
    return;
  }

  TextureDx12* textureDx12 = dynamic_cast<TextureDx12*>(texture);
  if (!textureDx12) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::clearDepthStencil - Invalid texture type");
    return;
  }

  // Get DSV handle for this mip/array slice
  D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = textureDx12->getDSVHandle(mipLevel, arrayLayer);
  if (dsvHandle.ptr == 0) {
    GlobalLogger::Log(LogLevel::Error, "CommandBufferDx12::clearDepthStencil - No valid DSV handle for texture");
    return;
  }

  // Determine if this is a depth-only or depth-stencil format
  bool hasStencil = g_isDepthFormat(textureDx12->getFormat()) && !g_isDepthOnlyFormat(textureDx12->getFormat());

  // Set clear flags based on format
  D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
  if (hasStencil) {
    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
  }

  // Transition texture to depth write state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = textureDx12->getResource();
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = textureDx12->getCurrentState();
  barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_DEPTH_WRITE;

  m_commandList_->ResourceBarrier(1, &barrier);

  // Clear the depth/stencil texture
  m_commandList_->ClearDepthStencilView(dsvHandle, clearFlags, depth, stencil, 0, nullptr);

  // Transition texture back to its original state
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
  barrier.Transition.StateAfter  = textureDx12->getCurrentState();
  m_commandList_->ResourceBarrier(1, &barrier);
}

void CommandBufferDx12::close() {
  if (!m_isRecording_) {
    return;
  }

  if (m_isRenderPassActive_) {
    endRenderPass();
  }

  m_commandList_->Close();
  m_isRecording_ = false;
}

//-------------------------------------------------------------------------
// CommandAllocatorManager implementation
//-------------------------------------------------------------------------

CommandAllocatorManager::~CommandAllocatorManager() {
  release();
}

bool CommandAllocatorManager::initialize(ID3D12Device* device, uint32_t allocatorCount) {
  if (!device) {
    GlobalLogger::Log(LogLevel::Error, "CommandAllocatorManager::initialize - Invalid device");
    return false;
  }

  release();

  m_device = device;

  // Create the initial pool of command allocators
  for (uint32_t i = 0; i < allocatorCount; ++i) {
    ComPtr<ID3D12CommandAllocator> allocator;
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));

    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error, "CommandAllocatorManager::initialize - Failed to create command allocator");
      return false;
    }

    m_availableAllocators.push_back(allocator);
  }

  return true;
}

void CommandAllocatorManager::release() {
  m_availableAllocators.clear();
  m_usedAllocators.clear();
  m_device = nullptr;
}

ID3D12CommandAllocator* CommandAllocatorManager::getCommandAllocator() {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "CommandAllocatorManager::getCommandAllocator - Manager not initialized");
    return nullptr;
  }

  // Try to reuse an existing allocator
  if (!m_availableAllocators.empty()) {
    ComPtr<ID3D12CommandAllocator> allocator = m_availableAllocators.back();
    m_availableAllocators.pop_back();

    // Reset the allocator so it's ready for use
    allocator->Reset();

    // Move to used list
    m_usedAllocators.push_back(allocator);

    return allocator.Get();
  }

  // If no allocators are available, create a new one
  ComPtr<ID3D12CommandAllocator> allocator;
  HRESULT hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error,
                      "CommandAllocatorManager::getCommandAllocator - Failed to create command allocator");
    return nullptr;
  }

  // Add to used list
  m_usedAllocators.push_back(allocator);

  return allocator.Get();
}

void CommandAllocatorManager::returnCommandAllocator(ID3D12CommandAllocator* allocator) {
  if (!allocator) {
    return;
  }

  // Find in used list
  for (auto it = m_usedAllocators.begin(); it != m_usedAllocators.end(); ++it) {
    if (it->Get() == allocator) {
      // Move to available list
      m_availableAllocators.push_back(*it);
      m_usedAllocators.erase(it);
      return;
    }
  }

  // If we reach here, the allocator wasn't in our used list
  GlobalLogger::Log(LogLevel::Warning,
                    "CommandAllocatorManager::returnCommandAllocator - Allocator not found in used list");
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12