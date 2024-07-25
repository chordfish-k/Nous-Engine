#include "pch.h"
#include "OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Nous {

    static GLenum ShaderTypeFromString(const std::string& type)
    {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel")
            return GL_FRAGMENT_SHADER;
        NS_CORE_ASSERT(false, "着色器类型未知！")
        return 0;
    }

    OpenGLShader::OpenGLShader(const std::string& path)
    {
        std::string source = ReadFile(path);
        auto shaderSrcs = PreProcess(source);
        Compile(shaderSrcs);

        // 从路径中提取名字
        fs::path filepath = path;
        m_Name = filepath.stem().string();
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
        : m_Name(name)
    {
        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertexSrc;
        sources[GL_FRAGMENT_SHADER] = fragmentSrc;
        Compile(sources);
    }

    OpenGLShader::~OpenGLShader()
    {
        glDeleteProgram(m_RendererID);
    }

    std::string OpenGLShader::ReadFile(const std::string& path)
    {
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
            NS_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "无效的 #type 标记");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            NS_CORE_ASSERT(nextLinePos != std::string::npos, "着色器语法错误！");
            pos = source.find(typeToken, nextLinePos);
            shaderSrc[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }
        return shaderSrc;
    }

    // https://www.khronos.org/opengl/wiki/Shader_Compilation#Example
    void OpenGLShader::Compile(std::unordered_map<GLenum, std::string>& shaderSources)
    {
        NS_CORE_ASSERT(shaderSources.size() <= 2, "着色器数量不能多于两个！");
        // 创建着色器程序
        GLenum program = glCreateProgram();
        std::array<GLenum, 2> glShaderIDs;
        // 编译各个着色器
        int glShaderIDIndex = 0;
        for (auto& kv: shaderSources)
        {
            GLenum type = kv.first;
            const std::string& src = kv.second;
            GLuint shader = glCreateShader(type);

            const GLchar* source = src.c_str();
            glShaderSource(shader, 1, &source, 0);

            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);

                NS_CORE_ERROR("{0}", infoLog.data());
                NS_CORE_ASSERT(false, "着色器编译失败！");
                return;
            }
            glAttachShader(program, shader);
            glShaderIDs[glShaderIDIndex++] = shader;
        }

        m_RendererID = program;

        // Link our m_RendererID
        glLinkProgram(m_RendererID);

        // Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*) &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

            // We don't need the m_RendererID anymore.
            glDeleteProgram(m_RendererID);
            // Don't leak shaders either.
            for (auto& id: glShaderIDs)
                glDeleteShader(id);

            // Use the infoLog as you see fit.
            NS_CORE_ERROR("{0}", infoLog.data());
            NS_CORE_ASSERT(false, "着色器连接失败！");
            // In this simple m_RendererID, we'll just leave
            return;
        }

        // Always detach shaders after a successful link.
        for (auto& id: glShaderIDs)
            glDeleteShader(id);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::UploadInt(const std::string& name, int value)
    {
        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform1i(location, value);
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

    void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
    {
        UploadFloat3(name, value);
    }

    void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
    {
        UploadFloat4(name, value);
    }

    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        UploadMat4(name, value);
    }
}