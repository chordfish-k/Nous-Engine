#include "pch.h"
#include "Layer.h"

#include "Application.h"

Nous::Layer::Layer(Application* application, const std::string& name)
        : m_Application(application), m_DebugName(name)
{
}

Nous::Layer::~Layer()
{
}

