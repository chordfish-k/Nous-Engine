#pragma once

#include "Core.h"
#include "Layer.h"

namespace Nous {

    // 用于管理各个层
    class NOUS_API LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
        std::vector<Layer*>::iterator end() { return m_Layers.end(); }

    private:
        std::vector<Layer*> m_Layers;
        unsigned int m_LayerInsertIndex = 0; // 分隔普通层和覆盖层
    };

}

