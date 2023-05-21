#pragma once


#include <string>

namespace GameEngine {

    class IShader {
    public:
        virtual ~IShader() = default;

        virtual bool Initialize(const std::string& shaderFile) = 0;
        virtual void Destroy() = 0;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;
    };

}  // namespace GameEngine
