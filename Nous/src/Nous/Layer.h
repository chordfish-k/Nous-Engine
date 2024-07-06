#pragma once

#include "Nous/Core.h"
#include "Nous/Event/Event.h"

namespace Nous {

    class NOUS_API Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttached() {};
        virtual void OnDetached() {};
        virtual void OnUpdate() {};
        virtual void OnEvent(Event& event) {};

        inline const std::string& GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

}
