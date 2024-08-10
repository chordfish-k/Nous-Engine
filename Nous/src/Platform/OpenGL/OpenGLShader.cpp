#include "pch.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Nous/Core/Timer.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>




namespace Nous {

    namespace Utils {

        static GLenum ShaderTypeFromString(const std::string& type)
        {
            if (type == "vertex")
                return GL_VERTEX_SHADER;
            if (type == "fragment" || type == "pixel")
                return GL_FRAGMENT_SHADER;
            NS_CORE_ASSERT(false, "着色器类型未知！")
            return 0;
        }

        static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
                case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
            }
            NS_CORE_ASSERT(false);
            return (shaderc_shader_kind)0;
        }

        static const char* GLShaderStageToString(GLenum stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
                case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
            }
            NS_CORE_ASSERT(false);
            return nullptr;
        }

        static const char* GetCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "assets/cache/shader/opengl";
        }

        static void CreateCacheDirectoryIfNeeded()
        {
            std::string cacheDirectory = GetCacheDirectory();
            if (!std::filesystem::exists(cacheDirectory))
                std::filesystem::create_directories(cacheDirectory);
        }

        static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
                case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
            }
            NS_CORE_ASSERT(false);
            return "";
        }

        static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
        {
            switch (stage)
            {
                case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
                case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
            }
            NS_CORE_ASSERT(false);
            return "";
        }
    }

    OpenGLShader::OpenGLShader(const std::string& path)
        : m_FilePath(path)
    {
        NS_PROFILE_FUNCTION();

        Utils::CreateCacheDirectoryIfNeeded();

        std::string source = ReadFile(path);
        auto shaderSrcs = PreProcess(source);
        {
            Timer timer;
            CompileOrGetVulkanBinaries(shaderSrcs);
            CompileOrGetOpenGLBinaries();
            CreateProgram();
            NS_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
        }
        // 从路径中提取名字
        fs::path filepath = path;
        m_Name = filepath.stem().string();
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
        : m_Name(name)
    {
        NS_PROFILE_FUNCTION();

        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertexSrc;
        sources[GL_FRAGMENT_SHADER] = fragmentSrc;

        CompileOrGetVulkanBinaries(sources);
        CompileOrGetOpenGLBinaries();
        CreateProgram();
    }

    OpenGLShader::~OpenGLShader()
    {
        NS_PROFILE_FUNCTION();

        glDeleteProgram(m_RendererID);
    }

    std::string OpenGLShader::ReadFile(const std::string& path)
    {
        NS_PROFILE_FUNCTION();

        std::string result;
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
        } else
        {
            NS_CORE_ERROR("无法打开文件 '{0}'", path);
        }
        return result;
    }

    std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
    {
        NS_PROFILE_FUNCTION();

        // 根据type分割两个着色器源码
        std::unordered_map<GLenum, std::string> shaderSrc;

        const char* typeToken = "#type";
        size_t typeTokenLen = strlen(typeToken);
        size_t pos = source.find(typeToken, 0);
        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos);
            NS_CORE_ASSERT(eol != std::string::npos, "着色器语法错误！")
            size_t begin = pos + typeTokenLen + 1;
            std::string type = source.substr(begin, eol - begin);
            NS_CORE_ASSERT(Utils::ShaderTypeFromString(type), "无效的 #type 标记");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            NS_CORE_ASSERT(nextLinePos != std::string::npos, "着色器语法错误！");
            pos = source.find(typeToken, nextLinePos);
            shaderSrc[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }
        return shaderSrc;
    }

    void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
    {
        GLuint program = glCreateProgram();

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        const bool optimize = true;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);

        std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

        auto& shaderData = m_VulkanSPIRV;
        shaderData.clear();
        for (auto&& [stage, source] : shaderSources)
        {
            std::filesystem::path shaderFilePath = m_FilePath;
            std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

            std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
            if (in.is_open())
            {
                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                in.seekg(0, std::ios::beg);

                auto& data = shaderData[stage];
                data.resize(size / sizeof(uint32_t));
                in.read((char*)data.data(), size);
            }
            else
            {
                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    NS_CORE_ERROR(module.GetErrorMessage());
                    NS_CORE_ASSERT(false);
                }

                shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

                std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
                if (out.is_open())
                {
                    auto& data = shaderData[stage];
                    out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                    out.flush();
                    out.close();
                }
            }
        }

        for (auto&& [stage, data]: shaderData)
            Reflect(stage, data);
    }

    void OpenGLShader::CompileOrGetOpenGLBinaries()
    {
        auto& shaderData = m_OpenGLSPIRV;

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        const bool optimize = false;
        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);

        std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

        shaderData.clear();
        m_OpenGLSourceCode.clear();
        for (auto&& [stage, spirv] : m_VulkanSPIRV)
        {
            std::filesystem::path shaderFilePath = m_FilePath;
            std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

            std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
            if (in.is_open())
            {
                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                in.seekg(0, std::ios::beg);

                auto& data = shaderData[stage];
                data.resize(size / sizeof(uint32_t));
                in.read((char*)data.data(), size);
            }
            else
            {
                spirv_cross::CompilerGLSL glslCompiler(spirv);
                m_OpenGLSourceCode[stage] = glslCompiler.compile();
                auto& source = m_OpenGLSourceCode[stage];

                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    NS_CORE_ERROR(module.GetErrorMessage());
                    NS_CORE_ASSERT(false);
                }

                shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

                std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
                if (out.is_open())
                {
                    auto& data = shaderData[stage];
                    out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                    out.flush();
                    out.close();
                }
            }
        }
    }

    void OpenGLShader::CreateProgram()
    {
        GLuint program = glCreateProgram();

        std::vector<GLuint> shaderIDs;
        for (auto&& [stage, spirv] : m_OpenGLSPIRV)
        {
            GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
            glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
            glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
            glAttachShader(program, shaderID);
        }

        glLinkProgram(program);

        GLint isLinked;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
            NS_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

            glDeleteProgram(program);

            for (auto id : shaderIDs)
                glDeleteShader(id);
        }

        for (auto id : shaderIDs)
        {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        m_RendererID = program;
    }

    void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
    {
        spirv_cross::Compiler compiler(shaderData);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        NS_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
        NS_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
        NS_CORE_TRACE("    {0} resources", resources.sampled_images.size());

        NS_CORE_TRACE("Uniform buffers:");
        for (const auto& resource : resources.uniform_buffers)
        {
            const auto& bufferType = compiler.get_type(resource.base_type_id);
            uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            int memberCount = bufferType.member_types.size();

            NS_CORE_TRACE("  {0}", resource.name);
            NS_CORE_TRACE("    Size = {0}", bufferSize);
            NS_CORE_TRACE("    Binding = {0}", binding);
            NS_CORE_TRACE("    Members = {0}", memberCount);
        }
    }

    void OpenGLShader::Bind() const
    {
        NS_PROFILE_FUNCTION();

        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        NS_PROFILE_FUNCTION();

        glUseProgram(0);
    }

    void OpenGLShader::UploadInt(const std::string& name, int value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1i(location, value);
    }

    void OpenGLShader::UploadIntArray(const std::string& name, int* values, uint32_t count)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1iv(location, count, values);
    }

    void OpenGLShader::UploadFloat(const std::string& name, float value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1f(location, value);
    }

    void OpenGLShader::UploadFloat2(const std::string& name, const glm::vec2& value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform2f(location, value.x, value.y);
    }

    void OpenGLShader::UploadFloat3(const std::string& name, const glm::vec3& value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform3f(location, value.x, value.y, value.z);
    }

    void OpenGLShader::UploadFloat4(const std::string& name, const glm::vec4& value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::UploadMat3(const std::string& name, const glm::mat3& matrix)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadMat4(const std::string& name, const glm::mat4& matrix)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::SetFloat(const std::string& name, float value)
    {
        NS_PROFILE_FUNCTION();

        UploadFloat(name, value);
    }

    void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
    {
        NS_PROFILE_FUNCTION();

        UploadFloat3(name, value);
    }

    void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
    {
        NS_PROFILE_FUNCTION();

        UploadFloat4(name, value);
    }

    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        NS_PROFILE_FUNCTION();

        UploadMat4(name, value);
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
        NS_PROFILE_FUNCTION();

        UploadInt(name, value);
    }

    void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
    {
        NS_PROFILE_FUNCTION();

        UploadIntArray(name, values, count);
    }
}