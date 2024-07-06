﻿#include "pch.h"
#include "LayerStack.h"

Nous::LayerStack::LayerStack()
{
    m_LayerInsert = m_Layers.begin();
}

Nous::LayerStack::~LayerStack()
{
    for (auto* layer : m_Layers)
        delete layer;
}

void Nous::LayerStack::PushLayer(Layer* layer)
{
    // 在最后一个非覆盖层后插入
    m_LayerInsert = m_Layers.emplace(m_LayerInsert, layer);
}

void Nous::LayerStack::PushOverlay(Layer* overlay)
{
    // 在列表最后插入
    m_Layers.emplace_back(overlay);
}

void Nous::LayerStack::PopLayer(Layer* layer)
{
    auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if (it != m_Layers.end())
    {
        m_Layers.erase(it);
        m_LayerInsert--;
    }
}

void Nous::LayerStack::PopOverlay(Layer* overlay)
{
    auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
    if (it != m_Layers.end())
    {
        m_Layers.erase(it);
    }
}