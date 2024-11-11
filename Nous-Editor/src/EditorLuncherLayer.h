#pragma once

#include "Nous.h"
#include "Nous/Core/Layer.h"

namespace Nous
{
    class EditorLuncherLayer : public Layer
    {
    public:
        EditorLuncherLayer();
        EditorLuncherLayer(Application* application, const ApplicationSpecification& spec);
        virtual ~EditorLuncherLayer() = default;

        virtual void OnAttached() override;
        virtual void OnDetached() override;

        virtual void OnUpdate(Timestep dt) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;
    private:
        ApplicationSpecification m_Spec;
    };
}

