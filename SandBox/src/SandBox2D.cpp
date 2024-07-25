#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include <chrono>

template<typename Fn>
class Timer
{
public:
    Timer(const char* name, Fn&& func)
        : m_Name(name), m_Func(func), m_Stopped(false)
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        if (!m_Stopped)
            Stop();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        m_Stopped = true;

        float duration = (end - start) * 0.001f;
        m_Func({ m_Name, duration });
    }
private:
    const char* m_Name;
    Fn m_Func;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    bool m_Stopped;
};

#define PROFILE_SCOPE(name) Timer __timer(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult);})

SandBox2D::SandBox2D()
    : Layer("SandBox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void SandBox2D::OnAttached()
{
    m_SquareVA = Nous::VertexArray::Create();
    float squareVertices[5 * 4] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };
    Nous::Ref<Nous::VertexBuffer> squareVB;
    squareVB = Nous::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
    squareVB->SetLayout(
        {
            {Nous::ShaderDataType::Float3, "a_Position"},
            {Nous::ShaderDataType::Float2, "a_TexCoord"}
        });
    m_SquareVA->AddVertexBuffer(squareVB);
    uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
    Nous::Ref<Nous::IndexBuffer> squareIB;
    squareIB = Nous::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    m_SquareVA->SetIndexBuffer(squareIB);

    m_FlatColorShader = Nous::Shader::Create("assets/shaders/FlatColor.glsl");

    m_MarioTexture = Nous::Texture2D::Create("assets/textures/Mario.png");
}

void SandBox2D::OnDetached()
{
}

void SandBox2D::OnUpdate(Nous::Timestep dt)
{
    PROFILE_SCOPE("SandBox2D::OnUpdate");
    // Update
    {
        PROFILE_SCOPE("CameraController::OnUpdate");
        m_CameraController.OnUpdate(dt);
    }

    // Render
    {
        PROFILE_SCOPE("Render Prepare");
        Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Nous::RenderCommand::Clear();
    }
    {
        PROFILE_SCOPE("Render Draw");
        Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
        Nous::Renderer2D::DrawQuad({0.0f, 0.0f}, {0.5f, 0.5f}, {0.8f, 0.2f, 0.3f, 1.0f});
        Nous::Renderer2D::DrawQuad({0.5f, 0.0f}, {0.5f, 0.5f}, {0.2f, 0.8f, 0.3f, 1.0f});
        Nous::Renderer2D::DrawQuad({0.5f, 0.5f}, {0.5f, 0.5f}, {0.2f, 0.3f, 0.8f, 1.0f});
        Nous::Renderer2D::DrawQuad({0.0f, 0.5f}, {0.5f, 0.5f}, m_MarioTexture);
        Nous::Renderer2D::EndScene();
    }
}

void SandBox2D::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

    for (auto& result : m_ProfileResults)
    {
        char label[50];
        strcpy_s(label, "%.3fms ");
        strcat_s(label, result.Name);
        ImGui::Text(label, result.Time);
    }
    m_ProfileResults.clear();

    ImGui::End();
}

void SandBox2D::OnEvent(Nous::Event& e)
{
    m_CameraController.OnEvent(e);
}