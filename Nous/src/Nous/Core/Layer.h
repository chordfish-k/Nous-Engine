#pragma once

#include "Core.h"
#include "Nous/Event/Event.h"
#include "Nous/Core/Timestep.h"

namespace Nous {

    class NOUS_API Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttached() {};
        virtual void OnDetached() {};
        virtual void OnUpdate(Timestep dt) {};
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event) {};

        const std::string& GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

}
