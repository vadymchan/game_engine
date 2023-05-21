#pragma once

#include <string>

namespace GameEngine {

    enum class ShaderType {
        VERTEX_SHADER,
        FRAGMENT_SHADER,
        GEOMETRY_SHADER,
        // add more types as needed...
    };


    template <typename DerivedShader>
    class IShader {
    public:
        bool Initialize(const std::string& shaderFile) {
            return static_cast<DerivedShader*>(this)->InitializeImplementation(shaderFile);
        }

        void Destroy() {
            return static_cast<DerivedShader*>(this)->DestroyImplementation();
        }

        void Bind() {
            return static_cast<DerivedShader*>(this)->BindImplementation();
        }

        void Unbind() {
            return static_cast<DerivedShader*>(this)->UnbindImplementation();
        }
    };

}  // namespace GameEngine

