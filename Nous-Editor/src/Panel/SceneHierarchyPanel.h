#pragma once

#include "Nous/Core/Base.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"

namespace Nous {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene>& scene);

        void SetContext(const Ref<Scene>& scene);

        void OnImGuiRender();
    private:
        template<typename T>
        void DisplayAddComponentEntry(const std::string& entryName);

        void DrawEntityNode(entt::entity entityID);
        void DrawComponents(Entity entity);
    private:
        Ref<Scene> m_Context;
    };

}
