#pragma once

#include "Nous.h"

class SandBox2D : public Nous::Layer
{
public:
    SandBox2D();
    virtual ~SandBox2D() = default;

    virtual void OnAttached() override;
    virtual void OnDetached() override;

    virtual void OnUpdate(Nous::Timestep dt) override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(Nous::Event& event) override;
private:
    Nous::CameraController m_CameraController;

    Nous::Ref<Nous::VertexArray> m_SquareVA;
    Nous::Ref<Nous::Shader> m_FlatColorShader;

    Nous::Ref<Nous::Texture2D> m_MarioTexture;

    glm::vec4 m_SquareColor = {0.2f, 0.3f, 0.8f, 1.0f};
};
