#include "pch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Nous {

    Ref <Shader> Shader::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLShader>(path);
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }

    Ref <Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: NS_CORE_ASSERT(false, "RendererAPI 未选择");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
        }
        NS_CORE_ASSERT(false, "RendererAPI 未知")
        return nullptr;
    }

    void ShaderLibrary::Add(const Ref <Shader>& shader)
    {
        auto& name = shader->GetName();
        Add(name, shader);
    }

    void ShaderLibrary::Add(const std::string& name, const Ref <Shader>& shader)
    {
        NS_CORE_ASSERT(!Exists(name), "着色器已经存在！");
        m_Shaders[name] = shader;
    }

    Ref <Shader> ShaderLibrary::Load(const std::string& path)
    {
        auto shader = Shader::Create(path);
        Add(shader);
        return shader;
    }

    Ref <Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
    {
        auto shader = Shader::Create(path);
        Add(name, shader);
        return shader;
    }

    Ref <Shader> ShaderLibrary::Get(const std::string& name)
    {
        NS_CORE_ASSERT(Exists(name), "着色器不存在！");
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const
    {
        return m_Shaders.find(name) != m_Shaders.end();
    }

}