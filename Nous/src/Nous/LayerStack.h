#pragma once

#include "Nous/Core.h"
#include "Nous/Layer.h"

namespace Nous {

    // 用于管理各个层
    class NOUS_API LayerStack
    {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
        std::vector<Layer*>::iterator end() { return m_Layers.end(); }

    private:
        std::vector<Layer*> m_Layers;
        std::vector<Layer*>::iterator m_LayerInsert; // 分隔普通层和覆盖层
    };

}

