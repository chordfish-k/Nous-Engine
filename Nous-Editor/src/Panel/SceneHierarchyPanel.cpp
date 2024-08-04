#include "SceneHierarchyPanel.h"

#include <imgui.h>

#include "Nous/Scene/Component.h"
#include <glm/gtc/type_ptr.hpp>

namespace Nous {

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
    {
        SetContent(scene);
    }

    void SceneHierarchyPanel::SetContent(const Ref<Scene>& scene)
    {
        m_Context = scene;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");
        m_Context->m_Registry.each([&](auto entityID)
        {
            Entity entity{entityID, m_Context.get()};
            DrawEntityNode(entity);
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_SelectedEntity = {};

        ImGui::End();

        // TODO 分离到新的类
        ImGui::Begin("Properties");
        if (m_SelectedEntity)
            DrawComponents(m_SelectedEntity);
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<CTag>();

        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)|ImGuiTreeNodeFlags_OpenOnArrow;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectedEntity = entity;
        }
        if (opened)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());
            if (opened)
            {
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<CTag>())
        {
            auto& tag = entity.GetComponent<CTag>();

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, tag.Tag.c_str());
            if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
            {
                tag.Tag = std::string(buffer);
            }
        }

        if (entity.HasComponent<CTransform>())
        {
            if (ImGui::TreeNodeEx((void*)typeid(CTransform).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
            {
                auto& transform = entity.GetComponent<CTransform>();

                ImGui::DragFloat3("Position", glm::value_ptr(transform.Transform[3]), 1.0f);
                ImGui::TreePop();
            }
        }
    }
}