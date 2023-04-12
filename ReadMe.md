## currently the structure of the project looks like that:
```
game_engine/
├─ include/
│  ├─ game_engine/
│  │  ├─ renderer/
│  │  │  ├─ render_api.h
│  │  │  ├─ dx11_render_api.h
│  │  │  ├─ dx12_render_api.h
│  │  │  └─ vulkan_render_api.h
│  │  └─ window/
│  │     └─ window.h
├─ src/
│  ├─ renderer/
│  │  ├─ render_api.cpp
│  │  ├─ dx11_render_api.cpp
│  │  ├─ dx12_render_api.cpp
│  │  └─ vulkan_render_api.cpp
│  └─ window/
│     └─ window.cpp
├─ CMakeLists.txt
└─ main.cpp
```