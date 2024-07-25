#pragma once

#include "Nous/Renderer/Shader.h"
#include <glm/glm.hpp>

// TODO 将来移除
typedef unsigned int GLenum;

namespace Nous {

    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& path);
        OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
        virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

        virtual const std::string& GetName() const override { return m_Name; }

        void UploadInt(const std::string& name, int value);
        void UploadFloat(const std::string& name, float value);
        void UploadFloat2(const std::string& name, const glm::vec2& value);
        void UploadFloat3(const std::string& name, const glm::vec3& value);
        void UploadFloat4(const std::string& name, const glm::vec4& value);
        void UploadMat3(const std::string& name, const glm::mat3& matrix);
        void UploadMat4(const std::string& name, const glm::mat4& matrix);

    private:
        std::string ReadFile(const std::string& path);
        std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
        void Compile(std::unordered_map<GLenum, std::string>& shaderSources);

    private:
        uint32_t m_RendererID;
        std::string m_Name;
    };

}
