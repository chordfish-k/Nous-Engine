#include "SceneHierarchyPanel.h"

#include "Nous/Scene/Component.h"

#include <imgui.h>
#include <imgui_internal.h>
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

        if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_Context->CreateEntity("Empty Entity");

            ImGui::EndPopup();
        }

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
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth; // 让一整行TreeNode都能够被点击
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectedEntity = entity;
        }

        // 右键 标记删除
        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
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

        // 处理删除
        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectedEntity == entity)
                m_SelectedEntity = {};
        }
    }


    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 4});
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            ImGui::PopStyleVar();

            if (ImGui::Button("x", { lineHeight, lineHeight }))
            {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove Component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
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
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag.Tag = std::string(buffer);
        }

        ImGui::SameLine();
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        if (ImGui::Button("Add Component", {ImGui::GetContentRegionAvailWidth(), lineHeight}))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            if (ImGui::MenuItem("Camera"))
            {
                m_SelectedEntity.AddComponent<CCamera>();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sprite Renderer"))
            {
                m_SelectedEntity.AddComponent<CSpriteRenderer>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }


        DrawComponent<CTransform>("Transform", entity, [](auto& component){
            DrawVec3Control("Position", component.Translation);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            DrawVec3Control("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale", component.Scale, 1.0f);
        });

        DrawComponent<CCamera>("Camera", entity, [](auto& component){
            auto& camera = component.Camera;
            ImGui::Checkbox("Primary", &component.Primary);

            const char* projectionTypeStrings[] = {"Perspective", "Orthographic"};
            const char* currentProjectionTypeString = projectionTypeStrings[(int) camera.GetProjectionType()];
            if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                    {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.SetProjectionType((SceneCamera::ProjectionType)i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float FOV = glm::degrees(camera.GetPerspFOV());
                if (ImGui::DragFloat("FOV", &FOV))
                    camera.SetPerspFOV(glm::radians(FOV));

                float perspNear = camera.GetPerspNearClip();
                if (ImGui::DragFloat("Near", &perspNear))
                    camera.SetPerspNearClip(perspNear);

                float perspFar = camera.GetPerspFarClip();
                if (ImGui::DragFloat("Far", &perspFar))
                    camera.SetPerspFarClip(perspFar);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthoSize();
                if (ImGui::DragFloat("Size", &orthoSize))
                    camera.SetOrthoSize(orthoSize);

                float orthoNear = camera.GetOrthoNearClip();
                if (ImGui::DragFloat("Near", &orthoNear))
                    camera.SetOrthoNearClip(orthoNear);

                float orthoFar = camera.GetOrthoFarClip();
                if (ImGui::DragFloat("Far", &orthoFar))
                    camera.SetOrthoFarClip(orthoFar);

                ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
            }
        });

        DrawComponent<CSpriteRenderer>("Sprite Renderer", entity, [](auto& component){
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
        });
    }
}