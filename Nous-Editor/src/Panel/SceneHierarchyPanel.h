#pragma once

#include "Nous/Core/Base.h"
#include "Nous/Core/Log.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"

namespace Nous {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene>& scene);

        void SetContent(const Ref<Scene>& scene);

        void OnImGuiRender();
    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
    private:
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };

}
