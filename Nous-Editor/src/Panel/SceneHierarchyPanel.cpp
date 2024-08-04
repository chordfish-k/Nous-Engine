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


    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
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
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
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

                DrawVec3Control("Position", transform.Translation);
                glm::vec3 rotation = glm::degrees(transform.Rotation);
                DrawVec3Control("Rotation", rotation);
                transform.Rotation = glm::radians(rotation);
                DrawVec3Control("Scale", transform.Scale, 1.0f);

                ImGui::TreePop();
            }
        }

        if (entity.HasComponent<CCamera>())
        {
            if (ImGui::TreeNodeEx((void*)typeid(CCamera).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
            {
                auto& cameraComponent = entity.GetComponent<CCamera>();
                auto& camera = cameraComponent.Camera;

                ImGui::Checkbox("Primary", &cameraComponent.Primary);

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

                    ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);
                }

                ImGui::TreePop();
            }
        }

        if (entity.HasComponent<CSpriteRenderer>())
        {
            if (ImGui::TreeNodeEx((void*)typeid(CSpriteRenderer).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer"))
            {
                auto& src = entity.GetComponent<CSpriteRenderer>();
                ImGui::ColorEdit4("Color", glm::value_ptr(src.Color));
                ImGui::TreePop();
            }
        }
    }
}