#pragma once

#include "Nous/Renderer/Shader.h"

namespace Nous {

    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);

        ~OpenGLShader();

        void Bind() const override;

        void Unbind() const override;

        void UploadFloat(const std::string& name, float value);

        void UploadFloat2(const std::string& name, const glm::vec2& value);

        void UploadFloat3(const std::string& name, const glm::vec3& value);

        void UploadFloat4(const std::string& name, const glm::vec4& value);

        void UploadMat3(const std::string& name, const glm::mat3& matrix);

        void UploadMat4(const std::string& name, const glm::mat4& matrix);

    private:
        uint32_t m_RendererID;
    };

}
