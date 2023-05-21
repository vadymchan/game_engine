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
│       ├── input
│       │   └── input_manager.h
│       ├── renderer
│       │   ├── dx11_renderer.h
│       │   ├── dx12_renderer.h
│       │   ├── vulkan_renderer.h
│       │   ├── dx11_device_context.h
│       │   ├── dx12_descriptor_heap.h
│       │   ├── i_renderer.h
│       │   └── render_api_factory.h
│       ├── resource
│       │   ├── resource_loader.h
│       │   └── resource_manager.h
│       ├── scene
│       │   └── scene_manager.h
│       ├── shader
│       │   ├── dx11_shader.h
│       │   ├── dx12_shader.h
│       │   ├── vulkan_shader.h
│       │   ├── i_shader.h
│       │   └── shader_factory.h
│       ├── window
│       │   ├── glfw_window.h
│       │   ├── winapi_window.h
│       │   ├── i_window.h
│       │   └── window_factory.h
│       └── logging
│           └── logger.h
├── main.cpp
└── src
    ├── application
    │   └── application.cpp
    ├── ecs
    │   ├── component
    │   │   └── component.cpp
    │   ├── entity
    │   │   └── entity.cpp
    │   └── system
    │       └── system.cpp
    ├── input
    │   └── input_manager.cpp
    ├── renderer
    │   ├── dx11_renderer.cpp
    │   ├── dx12_renderer.cpp
    │   ├── vulkan_renderer.cpp
    │   ├── dx11_device_context.cpp
    │   ├── dx12_descriptor_heap.cpp
    │   └── render_api_factory.cpp
    ├── resource
    │   ├── resource_loader.cpp
    │   └── resource_manager.cpp
    ├── scene
    │   └── scene_manager.cpp
    ├── shader
    │   ├── dx11_shader.cpp
    │   ├── dx12_shader.cpp
    │   ├── vulkan_shader.cpp
    │   └── shader_factory.cpp
    ├── window
    │   ├── glfw_window.cpp
    │   ├── winapi_window.cpp
    │   └── window_factory.cpp
    └── logging
        └── logger.cpp

```