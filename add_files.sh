#!/usr/bin/env bash

# -------------------------------------------------------------------------
# .gitignore
# -------------------------------------------------------------------------
git add .gitignore
git commit -m "Update .gitignore" \
           -m "Adjust ignore patterns to exclude new build artifacts and logs."

# -------------------------------------------------------------------------
# assets/shaders/forward_rendering
# -------------------------------------------------------------------------
git add assets/shaders/forward_rendering/shader.ps.hlsl
git commit -m "Refine forward rendering pixel shader" \
           -m "Tweaked lighting computations for improved color accuracy."

git add assets/shaders/forward_rendering/shader_instancing.vs.hlsl
git commit -m "Enhance forward rendering instancing vertex shader" \
           -m "Optimized instancing logic to reduce overhead and improve performance."

# -------------------------------------------------------------------------
# include/engine.h
# -------------------------------------------------------------------------
git add include/engine.h
git commit -m "Revise engine.h interface" \
           -m "Updated function declarations and documentation for clarity."

# -------------------------------------------------------------------------
# src/config/config.cpp
# -------------------------------------------------------------------------
git add src/config/config.cpp
git commit -m "Refactor config.cpp" \
           -m "Reorganized configuration loading for better maintainability."

# -------------------------------------------------------------------------
# src/game.cpp
# -------------------------------------------------------------------------
git add src/game.cpp
git commit -m "Update core game loop" \
           -m "Refined initialization steps and improved error handling."

# -------------------------------------------------------------------------
# src/gfx/renderer
# -------------------------------------------------------------------------
git add src/gfx/renderer/draw_command.cpp
git commit -m "Refine draw_command.cpp" \
           -m "Cleaned up rendering command logic and improved code readability."

git add src/gfx/renderer/material.cpp
git commit -m "Improve material.cpp" \
           -m "Extended material properties and refactored binding logic."

git add src/gfx/renderer/primitive_util.cpp
git commit -m "Enhance primitive_util.cpp" \
           -m "Optimized primitive creation routines for performance gains."

# -------------------------------------------------------------------------
# src/gfx/rhi/dx12
# -------------------------------------------------------------------------
git add src/gfx/rhi/dx12/buffer_dx12.cpp
git commit -m "Refactor buffer_dx12.cpp" \
           -m "Improved resource creation and memory alignment handling."

git add src/gfx/rhi/dx12/command_allocator_dx12.cpp
git commit -m "Adjust command_allocator_dx12.cpp" \
           -m "Optimized allocator reset and safety checks."

git add src/gfx/rhi/dx12/command_list_dx12.cpp
git commit -m "Update command_list_dx12.cpp" \
           -m "Refined command recording sequence for stability."

git add src/gfx/rhi/dx12/pipeline_state_info_dx12.cpp
git commit -m "Refine pipeline_state_info_dx12.cpp" \
           -m "Added missing validation checks and streamlined PSO creation."

git add src/gfx/rhi/dx12/render_frame_context_dx12.cpp
git commit -m "Revise render_frame_context_dx12.cpp" \
           -m "Cleaned up frame context transitions and resource barriers."

git add src/gfx/rhi/dx12/render_pass_dx12.cpp
git commit -m "Refactor render_pass_dx12.cpp" \
           -m "Unified pass setup flow and improved debug markers."

git add src/gfx/rhi/dx12/rhi_dx12.cpp
git commit -m "Update rhi_dx12.cpp" \
           -m "Added platform checks and refined device initialization logic."

git add src/gfx/rhi/dx12/rhi_type_dx12.cpp
git commit -m "Clean up rhi_type_dx12.cpp" \
           -m "Adjusted type conversions to handle extended formats."

git add src/gfx/rhi/dx12/shader_binding_instance_dx12.cpp
git commit -m "Improve shader_binding_instance_dx12.cpp" \
           -m "Expanded descriptor table setup and error reporting."

git add src/gfx/rhi/dx12/shader_binding_layout_dx12.cpp
git commit -m "Refine shader_binding_layout_dx12.cpp" \
           -m "Enhanced descriptor range management for robust binding."

git add src/gfx/rhi/dx12/swapchain_dx12.cpp
git commit -m "Revise swapchain_dx12.cpp" \
           -m "Improved swapchain resize handling and debugging output."

git add src/gfx/rhi/dx12/utils_dx12.cpp
git commit -m "Enhance utils_dx12.cpp" \
           -m "Extended helper functions for descriptor heap management."

# -------------------------------------------------------------------------
# src/gfx/rhi/rhi.cpp
# -------------------------------------------------------------------------
git add src/gfx/rhi/rhi.cpp
git commit -m "Revise rhi.cpp" \
           -m "Refined RHI initialization and cross-platform checks."

# -------------------------------------------------------------------------
# src/gfx/rhi/vulkan
# -------------------------------------------------------------------------
git add src/gfx/rhi/vulkan/command_pool_vk.cpp
git commit -m "Refactor command_pool_vk.cpp" \
           -m "Simplified command pool creation and reset logic."

git add src/gfx/rhi/vulkan/render_pass_vk.cpp
git commit -m "Enhance render_pass_vk.cpp" \
           -m "Streamlined Vulkan render pass setup for clarity."

git add src/gfx/rhi/vulkan/rhi_vk.cpp
git commit -m "Improve rhi_vk.cpp" \
           -m "Refactored Vulkan device initialization and resource management."

git add src/gfx/rhi/vulkan/utils_vk.cpp
git commit -m "Update utils_vk.cpp" \
           -m "Extended Vulkan utility helpers for pipeline debugging."

# -------------------------------------------------------------------------
# src/internal_include/event
# -------------------------------------------------------------------------
git add src/internal_include/event/keyboard_event_handler.h
git commit -m "Revise keyboard_event_handler.h" \
           -m "Improved key event dispatch and documentation."

git add src/internal_include/event/mouse_event_handler.h
git commit -m "Improve mouse_event_handler.h" \
           -m "Added new callback for multi-button tracking."

git add src/internal_include/event/window_event_handler.h
git commit -m "Enhance window_event_handler.h" \
           -m "Refined window resize events and focus change logic."

# -------------------------------------------------------------------------
# src/internal_include/file_loader
# -------------------------------------------------------------------------
git add src/internal_include/file_loader/image_file_loader.h
git commit -m "Update image_file_loader.h" \
           -m "Added new function for async image loading."

# -------------------------------------------------------------------------
# src/internal_include/game.h
# -------------------------------------------------------------------------
git add src/internal_include/game.h
git commit -m "Revise game.h" \
           -m "Streamlined core game class interface and event hooks."

# -------------------------------------------------------------------------
# src/internal_include/gfx/renderer
# -------------------------------------------------------------------------
git add src/internal_include/gfx/renderer/draw_command.h
git commit -m "Refine draw_command.h" \
           -m "Introduced new command struct for instanced rendering."

git add src/internal_include/gfx/renderer/material.h
git commit -m "Update material.h" \
           -m "Added new material parameters and shading modes."

git add src/internal_include/gfx/renderer/primitive_util.h
git commit -m "Improve primitive_util.h" \
           -m "Extended utility methods for mesh construction."

git add src/internal_include/gfx/renderer/renderer.h
git commit -m "Revise renderer.h" \
           -m "Unified rendering interface for multiple backends."

# -------------------------------------------------------------------------
# src/internal_include/gfx/rhi
# -------------------------------------------------------------------------
git add src/internal_include/gfx/rhi/buffer.h
git commit -m "Update buffer.h" \
           -m "Refactored buffer usage flags and size definitions."

git add src/internal_include/gfx/rhi/command_buffer_manager.h
git commit -m "Enhance command_buffer_manager.h" \
           -m "Added new methods to handle command buffer lifecycle."

# -------------------------------------------------------------------------
# src/internal_include/gfx/rhi/dx12
# -------------------------------------------------------------------------
git add src/internal_include/gfx/rhi/dx12/buffer_dx12.h
git commit -m "Refine buffer_dx12.h" \
           -m "Improved D3D12 buffer descriptor layout for alignment."

git add src/internal_include/gfx/rhi/dx12/command_allocator_dx12.h
git commit -m "Improve command_allocator_dx12.h" \
           -m "Added more robust error handling and reset methods."

git add src/internal_include/gfx/rhi/dx12/dxc_util.h
git commit -m "Update dxc_util.h" \
           -m "Enhanced DXC helper functions for shader compilation."

git add src/internal_include/gfx/rhi/dx12/pipeline_state_info_dx12.h
git commit -m "Revise pipeline_state_info_dx12.h" \
           -m "Streamlined PSO structures and added pipeline stats."

git add src/internal_include/gfx/rhi/dx12/render_frame_context_dx12.h
git commit -m "Refactor render_frame_context_dx12.h" \
           -m "Refined per-frame resource definitions and sync calls."

git add src/internal_include/gfx/rhi/dx12/render_pass_dx12.h
git commit -m "Enhance render_pass_dx12.h" \
           -m "Unified attachment descriptions for improved clarity."

git add src/internal_include/gfx/rhi/dx12/rhi_dx12.h
git commit -m "Update rhi_dx12.h" \
           -m "Added new DX12-specific methods and improved platform checks."

git add src/internal_include/gfx/rhi/dx12/rhi_type_dx12.h
git commit -m "Revise rhi_type_dx12.h" \
           -m "Expanded resource format enums to handle additional use cases."

git add src/internal_include/gfx/rhi/dx12/ring_buffer_dx12.h
git commit -m "Refactor ring_buffer_dx12.h" \
           -m "Simplified ring buffer logic and minimized overhead."

git add src/internal_include/gfx/rhi/dx12/shader_binding_instance_dx12.h
git commit -m "Improve shader_binding_instance_dx12.h" \
           -m "Enhanced descriptor binding to support multi-slot usage."

git add src/internal_include/gfx/rhi/dx12/texture_dx12.h
git commit -m "Update texture_dx12.h" \
           -m "Added resource state tracking and advanced texture layouts."

git add src/internal_include/gfx/rhi/dx12/utils_dx12.h
git commit -m "Enhance utils_dx12.h" \
           -m "Provided additional helper macros for D3D12 debugging."

# -------------------------------------------------------------------------
# src/internal_include/gfx/rhi
# -------------------------------------------------------------------------
git add src/internal_include/gfx/rhi/render_frame_context.h
git commit -m "Revise render_frame_context.h" \
           -m "Introduced new frame synchronization mechanisms."

git add src/internal_include/gfx/rhi/render_pass.h
git commit -m "Improve render_pass.h" \
           -m "Unified interface for multi-backend render passes."

git add src/internal_include/gfx/rhi/rhi.h
git commit -m "Update rhi.h" \
           -m "Added new RHI capabilities and clarified function docs."

git add src/internal_include/gfx/rhi/rhi_type.h
git commit -m "Refine rhi_type.h" \
           -m "Extended RHI enums for enhanced cross-platform support."

git add src/internal_include/gfx/rhi/shader_binding_layout.h
git commit -m "Improve shader_binding_layout.h" \
           -m "Defined consistent layout descriptors for all shader stages."

# -------------------------------------------------------------------------
# src/internal_include/gfx/rhi/vulkan
# -------------------------------------------------------------------------
git add src/internal_include/gfx/rhi/vulkan/command_pool_vk.h
git commit -m "Revise command_pool_vk.h" \
           -m "Adjusted flags and improved command buffer reuse."

git add src/internal_include/gfx/rhi/vulkan/render_frame_context_vk.h
git commit -m "Refactor render_frame_context_vk.h" \
           -m "Streamlined frame data structures for Vulkan usage."

git add src/internal_include/gfx/rhi/vulkan/render_pass_vk.h
git commit -m "Enhance render_pass_vk.h" \
           -m "Harmonized attachment configurations and load/store ops."

git add src/internal_include/gfx/rhi/vulkan/rhi_vk.h
git commit -m "Update rhi_vk.h" \
           -m "Added new Vulkan device management utilities."

git add src/internal_include/gfx/rhi/vulkan/swapchain_vk.h
git commit -m "Improve swapchain_vk.h" \
           -m "Refined swapchain creation parameters and synchronization."

git add src/internal_include/gfx/rhi/vulkan/utils_vk.h
git commit -m "Revise utils_vk.h" \
           -m "Augmented debug helper functions and pipeline creation helpers."

# -------------------------------------------------------------------------
# src/internal_include/gfx/scene
# -------------------------------------------------------------------------
git add src/internal_include/gfx/scene/render_object.h
git commit -m "Update render_object.h" \
           -m "Added new data fields for advanced object rendering."

git add src/internal_include/gfx/scene/view.h
git commit -m "Enhance view.h" \
           -m "Refined camera transformations and viewport definitions."

# -------------------------------------------------------------------------
# src/internal_include/input
# -------------------------------------------------------------------------
git add src/internal_include/input/input_manager.h
git commit -m "Refine input_manager.h" \
           -m "Reorganized input mapping and device polling methods."

git add src/internal_include/input/key.h
git commit -m "Update key.h" \
           -m "Defined new key codes and improved input enumerations."

# -------------------------------------------------------------------------
# src/internal_include/resources/image.h
# -------------------------------------------------------------------------
git add src/internal_include/resources/image.h
git commit -m "Enhance image.h" \
           -m "Introduced image metadata structures and pixel format support."

# -------------------------------------------------------------------------
# src/internal_include/utils/service/service_locator.h
# -------------------------------------------------------------------------
git add src/internal_include/utils/service/service_locator.h
git commit -m "Revise service_locator.h" \
           -m "Refactored service registration and retrieval logic."

# -------------------------------------------------------------------------
# src/internal_include/utils/third_party/stb_util.h
# -------------------------------------------------------------------------
git add src/internal_include/utils/third_party/stb_util.h
git commit -m "Improve stb_util.h" \
           -m "Extended STB wrapper functions for advanced image loading."

# -------------------------------------------------------------------------
# src/main.cpp
# -------------------------------------------------------------------------
git add src/main.cpp
git commit -m "Update main.cpp" \
           -m "Enhanced engine initialization sequence and logging."

# -------------------------------------------------------------------------
# src/utils/third_party/stb_util.cpp
# -------------------------------------------------------------------------
git add src/utils/third_party/stb_util.cpp
git commit -m "Refactor stb_util.cpp" \
           -m "Consolidated STB image handling and improved error checks."
