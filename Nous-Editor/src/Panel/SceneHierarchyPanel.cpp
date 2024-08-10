#include "SceneHierarchyPanel.h"
#include "Nous/Core/Application.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include <cstring>

#ifdef _MSVC_LANG
    #define _CRT_SECURE_NO_WARNINGS
#endif

namespace Nous {

    extern const std::filesystem::path g_AssetPath;

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
    {
        SetContext(scene);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
    {
        m_Context = scene;
        m_Context->SetSelectedEntity({});
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            m_Context->m_Registry.each([&](auto entityID)
                                       {
                                           Entity entity{entityID, m_Context.get()};
                                           DrawEntityNode(entity);
                                       });

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_Context->SetSelectedEntity({});

            if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_Context->CreateEntity("Empty Entity");

                ImGui::EndPopup();
            }
        }
        ImGui::End();

        // TODO 分离到新的类
        ImGui::Begin("Properties");
        if (m_Context && m_Context->GetSelectedEntity())
            DrawComponents(m_Context->GetSelectedEntity());
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<CTag>();

        ImGuiTreeNodeFlags flags = ((m_Context->GetSelectedEntity() == entity) ? ImGuiTreeNodeFlags_Selected : 0)|ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth; // 让一整行TreeNode都能够被点击
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_Context->SetSelectedEntity(entity);
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
            if (m_Context->GetSelectedEntity() == entity)
                m_Context->SetSelectedEntity({});
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
            strncpy(buffer, tag.Tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag.Tag = std::string(buffer);
        }

        ImGui::SameLine();
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        if (ImGui::Button("Add Component", {ImGui::GetContentRegionAvail().x, lineHeight}))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            DisplayAddComponentEntry<CCamera>("Camera");
            DisplayAddComponentEntry<CSpriteRenderer>("Sprite Renderer");
            DisplayAddComponentEntry<CCircleRenderer>("Circle Renderer");
            DisplayAddComponentEntry<CRigidbody2D>("Rigidbody 2D");
            DisplayAddComponentEntry<CBoxCollider2D>("Box Collider 2D");
            DisplayAddComponentEntry<CCircleCollider2D>("Circle Collider 2D");

            ImGui::EndPopup();
        }

        DrawComponent<CTransform>("Transform", entity, [](auto& component)
        {
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

        DrawComponent<CSpriteRenderer>("Sprite Renderer", entity, [](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

            ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
                    Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
                    if (texture->IsLoaded())
                        component.Texture = texture;
                    else
                        NS_WARN("无法加载纹理 {0}", texturePath.filename().string());
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
        });

        DrawComponent<CCircleRenderer>("Circle Renderer", entity, [](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
            ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
        });

        DrawComponent<CRigidbody2D>("Rigidbody 2D", entity, [](auto& component)
        {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int) component.Type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.Type = (CRigidbody2D::BodyType)i;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();

                ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
            }
        });

        DrawComponent<CBoxCollider2D>("Box Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        DrawComponent<CCircleCollider2D>("Circle Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat("Radius", &component.Radius);
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });
    }

    template<typename T>
    void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
        Entity selectedEntity = m_Context->GetSelectedEntity();
        if (!selectedEntity.HasComponent<T>())
        {
            if (ImGui::MenuItem(entryName.c_str()))
            {
                selectedEntity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }
}