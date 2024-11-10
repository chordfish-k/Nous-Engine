#pragma once

#include "Nous/Core/Base.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"

#include "Event/Observer.h"

namespace Nous {

    class SceneHierarchyPanel : public Observer
    {
    public:
        SceneHierarchyPanel();
        SceneHierarchyPanel(const Ref<Scene>& scene);

        void SetContext(const Ref<Scene>& scene);

        void OnImGuiRender();
        virtual void OnEditorEvent(EditorEvent& e) override;
    private:
        template<typename T>
        void DisplayAddComponentEntry(const std::string& entryName);

        bool DrawEntityNode(entt::entity entityID);
        void DrawComponents(Entity entity);

        void OnSavePrefab(SavePrefabEvent& e);
    private:
        Ref<Scene> m_Context;
    };

}
