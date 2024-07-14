#include <Nous.h>

#include "imgui/imgui.h"

class ExampleLayer : public Nous::Layer
{
public:
    ExampleLayer()
        : Layer("Example"), m_CameraPosition(0.0f)
    {
        // 初始化相机
        m_Camera.SetSize({1.6f, 0.9f});

        // 三角形
        m_VertexArray.reset(Nous::VertexArray::Create());
        float vertices[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
            0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
            0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
        };
        std::shared_ptr<Nous::VertexBuffer> vertexBuffer;
        vertexBuffer.reset(Nous::VertexBuffer::Create(vertices, sizeof(vertices)));
        Nous::BufferLayout layout = {
            {Nous::ShaderDataType::Float3, "a_Position"},
            {Nous::ShaderDataType::Float4, "a_Color"}
        };
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[3] = {0, 1, 2};
        std::shared_ptr<Nous::IndexBuffer> indexBuffer;
        indexBuffer.reset(Nous::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_VertexArray->SetIndexBuffer(indexBuffer);

        // 正方形
        m_SquareVA.reset(Nous::VertexArray::Create());
        float squareVertices[3 * 4] = {
            -0.75f, -0.75f, 0.0f,
            0.75f, -0.75f, 0.0f,
            0.75f, 0.75f, 0.0f,
            -0.75f, 0.75f, 0.0f
        };
        std::shared_ptr<Nous::VertexBuffer> squareVB;
        squareVB.reset(Nous::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout(
            {
                {Nous::ShaderDataType::Float3, "a_Position"}
            });
        m_SquareVA->AddVertexBuffer(squareVB);
        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<Nous::IndexBuffer> squareIB;
        squareIB.reset(Nous::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(squareIB);


        // 着色器
        std::string vertexSrc = R"(
            #version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

            uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}

        )";
        std::string fragmentSrc = R"(
            #version 330 core
			in vec3 v_Position;
			in vec4 v_Color;

            out vec4 color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}

        )";
        m_Shader.reset(new Nous::Shader(vertexSrc, fragmentSrc));

        std::string blueShaderVertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;

            uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

        std::string blueShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

        m_BlueShader.reset(new Nous::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
    }

    void OnUpdate(Nous::Timestep dt) override
    {
        if (Nous::Input::IsKeyPressed(NS_KEY_LEFT))
            m_CameraPosition.x -= m_CameraMoveSpeed * dt;
        else if (Nous::Input::IsKeyPressed(NS_KEY_RIGHT))
            m_CameraPosition.x += m_CameraMoveSpeed * dt;

        if (Nous::Input::IsKeyPressed(NS_KEY_UP))
            m_CameraPosition.y += m_CameraMoveSpeed * dt;
        else if (Nous::Input::IsKeyPressed(NS_KEY_DOWN))
            m_CameraPosition.y -= m_CameraMoveSpeed * dt;

        if (Nous::Input::IsKeyPressed(NS_KEY_A))
            m_CameraRotation += m_CameraRotationSpeed * dt;
        if (Nous::Input::IsKeyPressed(NS_KEY_D))
            m_CameraRotation -= m_CameraRotationSpeed * dt;

        Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Nous::RenderCommand::Clear();

        m_Camera.SetPosition(m_CameraPosition);
        m_Camera.SetRotation(m_CameraRotation);

        Nous::Renderer::BeginScene(m_Camera);

        Nous::Renderer::Submit(m_BlueShader, m_SquareVA);
        Nous::Renderer::Submit(m_Shader, m_VertexArray);

        Nous::Renderer::EndScene();
    }

    virtual void OnImGuiRender() override
    {
    }

    void OnEvent(Nous::Event& event) override
    {
    }

private:
    std::shared_ptr<Nous::Shader> m_Shader;
    std::shared_ptr<Nous::VertexArray> m_VertexArray;

    std::shared_ptr<Nous::Shader> m_BlueShader;
    std::shared_ptr<Nous::VertexArray> m_SquareVA;

    Nous::Camera m_Camera;
    glm::vec3 m_CameraPosition;
    float m_CameraMoveSpeed = 5.0f;
    float m_CameraRotation = 0.0f;
    float m_CameraRotationSpeed = 180.0f;
};

class Sandbox : public Nous::Application
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
    }

    ~Sandbox()
    {

    }
};

// 使用Nous的程序入口点，提供一个创建应用的函数
Nous::Application* Nous::CreateApplication()
{
    return new Sandbox();
}