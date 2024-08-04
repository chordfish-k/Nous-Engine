#pragma once

#include "Nous.h"

class ExampleLayer : public Nous::Layer
{
public:
    ExampleLayer();

    void OnUpdate(Nous::Timestep dt) override;

    virtual void OnImGuiRender() override;

    void OnEvent(Nous::Event& e) override;

private:
    Nous::ShaderLibrary m_ShaderLibrary;
    Nous::Ref<Nous::Shader> m_Shader;
    Nous::Ref<Nous::VertexArray> m_VertexArray;

    Nous::Ref<Nous::Shader> m_FlatColorShader;
    Nous::Ref<Nous::VertexArray> m_SquareVA;

    Nous::Ref<Nous::Texture2D> m_Texture;
    Nous::Ref<Nous::Texture2D> m_MarioTexture;

    Nous::OrthoCameraController m_CameraController;
    glm::vec3 m_SquareColor = {0.2f, 0.3f, 0.8f};
};
