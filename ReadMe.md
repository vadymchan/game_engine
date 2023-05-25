## The target architecture for the first triangle (draft):
```
game_engine
├── include
│   └── game_engine
│       ├── application
│       │   └── application.h
│       ├── ecs
│       │   ├── component
│       │   │   └── component.h
│       │   ├── entity
│       │   │   └── entity.h
│       │   └── system
│       │       └── system.h
│       ├── renderer
│       │   ├── api_specific
│       │   │   ├── dx11
│       │   │   │   ├── shader
│       │   │   │   │   ├── dx11_vertex_shader.h
│       │   │   │   │   ├── dx11_fragment_shader.h
│       │   │   │   │   ├── dx11_geometry_shader.h
│       │   │   │   │   ├── dx11_hull_shader.h
│       │   │   │   │   ├── dx11_domain_shader.h
│       │   │   │   │   ├── dx11_compute_shader.h
│       │   │   │   │   └── dx11_shader_program.h
│       │   │   │   ├── device
│       │   │   │   │   ├── dx11_device.h
│       │   │   │   │   ├── dx11_swap_chain.h
│       │   │   │   │   └── dx11_render_target_view.h
│       │   │   │   ├── input
│       │   │   │   │   ├── dx11_viewport.h
│       │   │   │   │   └── dx11_input_layout.h
│       │   │   │   ├── buffer
│       │   │   │   │   ├── dx11_buffer.h
│       │   │   │   │   ├── dx11_depth_stencil_view.h
│       │   │   │   │   └── dx11_depth_stencil_state.h
│       │   │   ├── dx12
│       │   │   │   ├── shader
│       │   │   │   │   ├── dx12_vertex_shader.h
│       │   │   │   │   ├── dx12_fragment_shader.h
│       │   │   │   │   ├── dx12_geometry_shader.h
│       │   │   │   │   ├── dx12_hull_shader.h
│       │   │   │   │   ├── dx12_domain_shader.h
│       │   │   │   │   ├── dx12_compute_shader.h
│       │   │   │   │   └── dx12_shader_program.h
│       │   │   │   ├── device
│       │   │   │   │   ├── dx12_device.h
│       │   │   │   │   ├── dx12_swap_chain.h
│       │   │   │   │   ├── dx12_descriptor_heap.h
│       │   │   │   │   └── dx12_resource.h
│       │   │   │   ├── command
│       │   │   │   │   ├── dx12_command_allocator.h
│       │   │   │   │   └── dx12_command_queue.h
│       │   │   │   ├── pipeline
│       │   │   │   │   ├── dx12_root_signature.h
│       │   │   │   │   └── dx12_pipeline_state.h
│       │   │   │   └── buffer
│       │   │   │       ├── dx12_vertex_buffer_view.h
│       │   │   │       ├── dx12_depth_stencil_view.h
│       │   │   │       └── dx12_depth_stencil_state.h
│       │   │   ├── vk
│       │   │   │   ├── shader
│       │   │   │   │   ├── vk_vertex_shader.h
│       │   │   │   │   ├── vk_fragment_shader.h
│       │   │   │   │   ├── vk_geometry_shader.h
│       │   │   │   │   ├── vk_tessellation_control_shader.h
│       │   │   │   │   ├── vk_tessellation_evaluation_shader.h
│       │   │   │   │   ├── vk_compute_shader.h
│       │   │   │   │   └── vk_shader_program.h
│       │   │   │   ├── device
│       │   │   │   │   ├── vk_device.h
│       │   │   │   │   ├── vk_swap_chain.h
│       │   │   │   │   └── vk_render_pass.h
│       │   │   │   ├── pipeline
│       │   │   │   │   ├── vk_pipeline_layout.h
│       │   │   │   │   └── vk_graphics_pipeline.h
│       │   │   │   ├── command
│       │   │   │   │   ├── vk_command_pool.h
│       │   │   │   │   └── vk_command_buffer.h
│       │   │   │   ├── buffer
│       │   │   │   │   ├── vk_buffer.h
│       │   │   │   │   └── vk_image.h
│       │   │   │   └── descriptor
│       │   │   │       ├── vk_descriptor_set_layout.h
│       │   │   │       ├── vk_descriptor_pool.h
│       │   │   │       └── vk_descriptor_set.h
│       │   │   ├── opengl
│       │   │   │   ├── shader
│       │   │   │	|	 ├── gl_vertex_shader.h
│       │   │   │	|	 ├── gl_fragment_shader.h
│       │   │   │  	|	 ├── gl_geometry_shader.h
│       │   │   │	|	 ├── gl_tessellation_control_shader.h
│       │   │   │	|	 ├── gl_tessellation_evaluation_shader.h
│       │   │   │	|	 ├── gl_compute_shader.h
│       │   │   │	|	 └── gl_shader_program.h
│       │   │   │   ├── device
│       │   │   │   │   ├── gl_device.h
│       │   │   │   │   ├── gl_context.h
│       │   │   │   │   ├── gl_framebuffer.h
│       │   │   │   ├── buffer
│       │   │   │   │   ├── gl_vertex_buffer.h
│       │   │   │   │   ├── gl_index_buffer.h
│       │   │   │   ├── texture
│       │   │   │   │   ├── gl_texture_2d.h
│       │   │   │   │   ├── gl_texture_cube_map.h
│       │   │   │   └── state
│       │   │   │		├── gl_blend_state.h
│       │   │   │		├── gl_depth_stencil_state.h
│       │   │   │		└── gl_rasterizer_state.h
│       │   │   └── interfaces
│       │   │       ├── i_device.h
│	    │   │       ├── i_swap_chain.h
│	    │   │       ├── i_shader.h
│	    │   │       └── i_buffer.h
│       │   └── renderer_factory.h
│       ├── window
│       │    ├── i_window.h
│       │    ├── glfw_window.h
│       │    ├── winapi_window.h
│       │    └── window_factory.h
│       └── utilities
│           ├── log.h
│           ├── timer.h
│           └── config.h
└── src ...

```

P.S this is not final tree hirerarchy, i will update it as i implement first triangle.

## naming conventions for this project:
 
| Code Element | Naming Convention | Example |
| --- | --- | --- |
| Classes | PascalCase | `GameEngine` |
| Structures | PascalCase | `Vector2D` |
| Functions / Methods | camelCase | `updatePosition()` |
| Public Variables | camelCase | `playerHealth` |
| Member Variables | `m_` prefix + camelCase | `m_position` |
| Private Member Variables | `m_` prefix + camelCase + `_` postfix | `m_position_` | 
| Private Methods | camelCase + `_` postfix | `updatePosition_()` | 
| Constants | `k` prefix + PascalCase | `kMaxPlayers` | - i've seen it in Google's C++ style guide
| Enums | PascalCase for type, UPPER_CASE_WITH_UNDERSCORES for values | `enum class Difficulty { EASY, MEDIUM, HARD };` |
| Namespaces | lowercase with underscores | `game_logic` |
| Interface Classes | `I` prefix + PascalCase | `ICollidable` |
| Boolean Variables | `is` or `has` prefix + camelCase | `isVisible`, `hasPowerUp` |
| Template Parameters | Single uppercase letters | `template <class T>` |
| File Names | lowercase with underscores, match class name | `game_engine.h` |
| Macros | UPPER_CASE_WITH_UNDERSCORES | `#define MAX_HEALTH 100` |
| Typedefs and Type Aliases | PascalCase | `typedef long int BigNum;` or `using BigNum = long int;` |
| Global Variables | g_ prefix + camelCase | `g_gameState` |
| Static Variables | s_ prefix + camelCase | `s_instanceCount` |

P.S if the static variable is an member of the class (struct), then the priority will be given to the `s_` prefix.

P.S.S I'm still finding the best naming convention for this project, so it may change in the future. (feel free to discuss about this topic and propose your variants)
- For now, i wondering about const static variables (for me, using something like `ks_maxHealth` is really weird and i'm not sure if this is right).
- Also, mayyybe in the future i will switch naming convention for const variable to `UPPER_CASE_WITH_UNDERSCORES`
