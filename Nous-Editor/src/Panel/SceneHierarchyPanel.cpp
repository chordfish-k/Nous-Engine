#include "SceneHierarchyPanel.h"

#include "Nous/Core/Application.h"
#include "Nous/Asset/AssetManager.h"
#include "Nous/Asset/AssetMetadata.h"
#include "Nous/Scene/SceneSerializer.h"
#include "Nous/Scene/System/TransformSystem.h"
#include "Nous/Script/ScriptEngine.h"
#include "Nous/Script/ScriptGlue.h"
#include "Nous/Asset/TextureImporter.h"
#include "Nous/Anim/AnimClip.h"
#include "Nous/UI/UI.h"


#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <cstring>

#ifdef _MSVC_LANG
    #define _CRT_SECURE_NO_WARNINGS
#endif

namespace Nous {
    SceneHierarchyPanel::SceneHierarchyPanel()
        : SceneHierarchyPanel(nullptr)
    {
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
    {
        SetContext(scene);
        EditorEventEmitter::AddObserver(this);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
    {
        m_Context = scene;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            if (ImGui::Button("Create Entity", { ImGui::GetContentRegionAvailWidth(), lineHeight }))
            {
                Entity entity = m_Context->CreateEntity("Empty Entity");
                m_Context->m_RootEntityMap[entity.GetUUID()] = entity;
                m_Context->SetSelectedEntity(entity);
            }

            ImGui::BeginChild("Nodes");
            for (auto& [uuid, entityID] : m_Context->m_RootEntityMap)
            {
                if (DrawEntityNode(entityID))
                    break;
            }

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_Context->SetSelectedEntity({});
            ImGui::EndChild();
        }
        ImGui::End();



        // TODO 分离到新的类
        ImGui::Begin("Properties");
        if (m_Context && m_Context->GetSelectedEntity())
            DrawComponents(m_Context->GetSelectedEntity());
        ImGui::End();
    }

    void SceneHierarchyPanel::OnEditorEvent(EditorEvent& e)
    {
        EditorEventDispatcher dispatcher(e);
        dispatcher.Dispatch<SavePrefabEvent>(NS_BIND_EVENT_FN(SceneHierarchyPanel::OnSavePrefab));
    }

    bool SceneHierarchyPanel::DrawEntityNode(entt::entity entityID)
    {
        bool changed = false;

        Entity entity{ entityID, m_Context.get() };
        auto& tag = entity.GetComponent<CTag>();
        auto& transform = entity.GetComponent<CTransform>();

        bool canBeOpen = transform.Children.size() > 0 && !transform.HideChild;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;// 让一整行TreeNode都能够被点击
        if (m_Context->GetSelectedEntity() == entity) flags |= ImGuiTreeNodeFlags_Selected;
        if (canBeOpen) flags |= ImGuiTreeNodeFlags_OpenOnArrow;
        else flags |= ImGuiTreeNodeFlags_Leaf;
        if (transform.Open && canBeOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;

        transform.Open = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s%s", tag.Tag.c_str(), transform.PrefabAsset ? "[Prefab]" : "");
        if (ImGui::IsItemClicked())
        {
            m_Context->SetSelectedEntity(entity);
        }

        // 右键
        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            // 标记删除
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            // 移动到根
            if (ImGui::MenuItem("Move to Root"))
            {
                UUID idSource = entity.GetUUID();
                auto& transformSource = m_Context->GetEntityByUUID(idSource).GetTransform();
                if (transformSource.Parent)
                {
                    auto& parent = m_Context->GetEntityByUUID(transformSource.Parent).GetTransform();
                    parent.Children.erase(std::find(parent.Children.begin(), parent.Children.end(), idSource));

                    transformSource.Parent = 0;
                    transformSource.ParentTransform = glm::mat4(1.0f);
                    m_Context->m_RootEntityMap[idSource] = entity;

                    TransformSystem::SetSubtreeDirty(m_Context.get(), entityID);

                    changed = true;
                }
            }

            if (transform.PrefabAsset)
            {
                if (ImGui::MenuItem("Toggle Children"))
                    transform.HideChild = !transform.HideChild;
            }
            ImGui::EndPopup();
        }

        // 拖动
        if (ImGui::BeginDragDropSource())
        {
            UUID id = entity.GetUUID();
            ImGui::SetDragDropPayload("TRANSFORM_NODE", &id, sizeof(id));
            ImGui::Text("%s", tag.Tag.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget() && transform.PrefabAsset == 0)
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TRANSFORM_NODE"))
            {
                UUID idTarget = entity.GetUUID();
                UUID idSource = *(UUID*)payload->Data;
                if (idTarget != idSource)
                {
                    auto& transformTarget = transform;
                    Entity eSource = m_Context->GetEntityByUUID(idSource);
                    auto& transformSource = eSource.GetTransform();
                    if (transformSource.Parent)
                    {
                        auto& parent = m_Context->GetEntityByUUID(transformSource.Parent).GetTransform();
                        parent.Children.erase(std::find(parent.Children.begin(), parent.Children.end(), idSource));
                    }
                    else
                    {
                        m_Context->m_RootEntityMap.erase(idSource);
                    }
                    transformTarget.Children.push_back(idSource);
                    transformSource.Parent = idTarget;

                    TransformSystem::SetSubtreeDirty(m_Context.get(), eSource);

                    changed = true;
                }
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_BROWSER_ITEM"))
            {
                UUID idTarget = entity.GetUUID();
                AssetHandle handle_ = *(AssetHandle*)payload->Data;
                if (AssetManager::GetAssetType(handle_) == AssetType::Prefab)
                {
                    // 当作Prefab嵌入
                    // 反序列化
                    SceneSerializer serializer(m_Context);
                    serializer.DeserializePrefabTo(handle_, idTarget);
                    // 保存资源句柄
                    
                    changed = true;
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (transform.Open)
        {
            if (canBeOpen)
            {
                for (auto& uid : transform.Children)
                {
                    Entity entityChild = m_Context->GetEntityByUUID(uid);
                    if (DrawEntityNode(entityChild))
                        break;
                }
            }

            ImGui::TreePop();
        }

        // 处理删除
        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_Context->GetSelectedEntity() == entity)
                m_Context->SetSelectedEntity({});
            changed = true;
        }

        return changed;
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen 
            | ImGuiTreeNodeFlags_Framed 
            | ImGuiTreeNodeFlags_SpanAvailWidth 
            | ImGuiTreeNodeFlags_AllowItemOverlap 
            | ImGuiTreeNodeFlags_FramePadding;
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
        // Name
        {
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            auto& tag = entity.GetComponent<CTag>();
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tag.Tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag.Tag = std::string(buffer);
        }
        
        // ID
        {
            ImGui::Text("UUID:%llu", entity.GetUUID());
        }

        ImGui::SameLine();

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        if (ImGui::Button("Add Component", {ImGui::GetContentRegionAvail().x, lineHeight}))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            DisplayAddComponentEntry<CCamera>("CCamera");
            DisplayAddComponentEntry<CMonoScript>("CMonoScript");
            DisplayAddComponentEntry<CSpriteRenderer>("CSpriteRenderer");
            DisplayAddComponentEntry<CCircleRenderer>("CCircleRenderer");
            DisplayAddComponentEntry<CRigidbody2D>("CRigidbody2D");
            DisplayAddComponentEntry<CBoxCollider2D>("CBoxCollider2D");
            DisplayAddComponentEntry<CCircleCollider2D>("CCircleCollider2D");
            DisplayAddComponentEntry<CTextRenderer>("CTextRenderer");
            DisplayAddComponentEntry<CAnimPlayer>("CAnimPlayer");

            ImGui::EndPopup();
        }

        DrawComponent<CTransform>("CTransform", entity, [&](auto& component)
        {
            if (component.PrefabAsset)
            {
                float width = ImGui::GetContentRegionAvailWidth();
                if (ImGui::Button("Reload Prefab", {width, 0}))
                {
                    NS_TRACE("Reload Prefab {}", component.PrefabAsset);
                }
            }
            
            bool changed = false;

            if (UI::DrawVec3Control("Position", component.Translation)) changed = true;
            glm::vec3 rotation = glm::degrees(component.Rotation);
            if (UI::DrawVec3Control("Rotation", rotation)) changed = true;
            component.Rotation = glm::radians(rotation);
            if (UI::DrawVec3Control("Scale", component.Scale, 1.0f)) changed = true;

            if (changed)
                TransformSystem::SetSubtreeDirty(m_Context.get(), entity);
        });

        DrawComponent<CCamera>("CCamera", entity, [](auto& component){
            auto& camera = component.Camera;
            UI::DrawCheckbox("Primary", &component.Primary);

            const char* projectionTypeStrings[] = {"Perspective", "Orthographic"};
            uint32_t currentIndex = (int)camera.GetProjectionType();
            if (UI::DrawCombo("Projection", projectionTypeStrings, &currentIndex, 2))
            {
                camera.SetProjectionType((SceneCamera::ProjectionType)currentIndex);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float FOV = glm::degrees(camera.GetPerspFOV());
                if (UI::DrawFloatControl("FOV", &FOV))
                    camera.SetPerspFOV(glm::radians(FOV));

                float perspNear = camera.GetPerspNearClip();
                if (UI::DrawFloatControl("Near", &perspNear))
                    camera.SetPerspNearClip(perspNear);

                float perspFar = camera.GetPerspFarClip();
                if (UI::DrawFloatControl("Far", &perspFar))
                    camera.SetPerspFarClip(perspFar);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthoSize();
                if (UI::DrawFloatControl("Size", &orthoSize))
                    camera.SetOrthoSize(orthoSize);

                float orthoNear = camera.GetOrthoNearClip();
                if (UI::DrawFloatControl("Near", &orthoNear))
                    camera.SetOrthoNearClip(orthoNear);

                float orthoFar = camera.GetOrthoFarClip();
                if (UI::DrawFloatControl("Far", &orthoFar))
                    camera.SetOrthoFarClip(orthoFar);

                UI::DrawCheckbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
            }
        });

        DrawComponent<CMonoScript>("CMonoScript", entity, [entity, scene = m_Context](auto& component) mutable // mutable:允许修改捕获的变量
        {
            bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

            static char buffer[64];
            strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

            // Class 脚本类名
            {
                UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);
                if (UI::DrawInputText("Class", buffer, sizeof(buffer)))
                {
                    component.ClassName = buffer;
                    return;
                }
            }
            

            // 字段 Fields
            bool sceneRunning = scene->IsRunning();
            if (sceneRunning)
            {
                Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
                if (scriptInstance)
                {
                    const auto& fields = scriptInstance->GetScriptClass()->GetFields();
                    for (const auto& [name, field] : fields)
                    {
                        if (field.Type == ScriptFieldType::Float)
                        {
                            auto data = scriptInstance->GetFieldValue<float>(name);
                            if (UI::DrawFloatControl(name, &data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        if (field.Type == ScriptFieldType::Vector2)
                        {
                            auto data = scriptInstance->GetFieldValue<glm::vec2>(name);
                            if (UI::DrawVec2Control(name, data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            auto data = scriptInstance->GetFieldValue<bool>(name);
                            if (UI::DrawCheckbox(name, &data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Prefab)
                        {
                            auto data = scriptInstance->GetFieldValue<AssetHandleWrapper>(name);
                            AssetHandle handle = data.Handle;
                            if (UI::DrawAssetDragDropBox(name, AssetManager::GetAssetFileName(handle), &handle, AssetType::Prefab))
                            {
                                data.Handle = handle;
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                    }
                }
            }
            else if (scriptClassExists)
            {
                Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
                const auto& fields = entityClass->GetFields();
                auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

                for (const auto& [name, field] : fields)
                {
                    // 如果该字段已经缓存
                    if (entityFields.find(name) != entityFields.end())
                    {
                        ScriptFieldInstance& scriptField = entityFields.at(name);

                        // 
                        if (field.Type == ScriptFieldType::Float)
                        {
                            auto data = scriptField.GetValue<float>();
                            if (UI::DrawFloatControl(name, &data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Vector2)
                        {
                            auto data = scriptField.GetValue<glm::vec2>();
                            if (UI::DrawVec2Control(name, data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            auto data = scriptField.GetValue<bool>();
                            if (UI::DrawCheckbox(name, &data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Prefab)
                        {
                            auto data = scriptField.GetValue<AssetHandleWrapper>();
                            AssetHandle handle = data.Handle;
                            if (UI::DrawAssetDragDropBox(name, AssetManager::GetAssetFileName(handle), &handle, AssetType::Prefab))
                            {
                                data.Handle = handle;
                                scriptField.SetValue(data);
                            }
                        }
                    }
                    else
                    {
                        if (field.Type == ScriptFieldType::Float)
                        {
                            float data = 0.0f;
                            if (UI::DrawFloatControl(name, &data))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                scriptField.SetValue(data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Vector2)
                        {
                            glm::vec2 data(0.0f);
                            if (UI::DrawVec2Control(name, data))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                scriptField.SetValue(data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            bool data = false;
                            if (UI::DrawCheckbox(name, &data))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                scriptField.SetValue(data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Prefab)
                        {
                            AssetHandleWrapper data{0};
                            AssetHandle handle = data.Handle;
                            if (UI::DrawAssetDragDropBox(name, AssetManager::GetAssetFileName(handle), &handle, AssetType::Prefab))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                data.Handle = handle;
                                scriptField.SetValue(data);
                            }
                        }
                    }
                }
            }
        });

        DrawComponent<CSpriteRenderer>("CSpriteRenderer", entity, [](auto& component)
        {
            UI::DrawColor4Control("Color", component.Color);
            
            std::string btnLabel = "None";
            bool isTextureValid = false;
            if (component.Texture != 0)
            {
                btnLabel = AssetManager::GetAssetFileName(component.Texture);
            }

            AssetHandle handle = component.Texture;
            if (UI::DrawAssetDragDropBox("Texture", btnLabel, &handle, AssetType::Texture2D))
            {
                component.Texture = handle;
            }

            UI::DrawFloatControl("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
            UI::DrawIntControl("Sheet Width", &component.SheetWidth, 0);
            UI::DrawIntControl("Sheet Height", &component.SheetHeight, 0);
            UI::DrawIntControl("Index", &component.Index, 0);

        });

        DrawComponent<CCircleRenderer>("CCircleRenderer", entity, [](auto& component)
        {
            UI::DrawColor4Control("Color", component.Color);
            UI::DrawFloatControl("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
            UI::DrawFloatControl("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
        });

        DrawComponent<CRigidbody2D>("CRigidbody2D", entity, [](auto& component)
        {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            uint32_t currentIndex = (int) component.Type;
            if (UI::DrawCombo("Body Type", bodyTypeStrings, &currentIndex, 3))
            {
                component.Type = (CRigidbody2D::BodyType)currentIndex;
            }

            UI::DrawCheckbox("Fixed Rotation", &component.FixedRotation);
        });

        DrawComponent<CBoxCollider2D>("CBoxCollider2D", entity, [](auto& component)
        {
            UI::DrawVec2Control("Offset", component.Offset);
            UI::DrawVec2Control("Size", component.Size);
            UI::DrawFloatControl("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            UI::DrawFloatControl("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            UI::DrawFloatControl("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            UI::DrawFloatControl("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        DrawComponent<CCircleCollider2D>("CCircleCollider2D", entity, [](auto& component)
        {
            UI::DrawVec2Control("Offset", component.Offset);
            UI::DrawFloatControl("Radius", &component.Radius);
            UI::DrawFloatControl("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            UI::DrawFloatControl("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            UI::DrawFloatControl("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            UI::DrawFloatControl("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        DrawComponent<CTextRenderer>("CTextRenderer", entity, [](auto& component)
        {
            UI::DrawInputTextMultiline("Text String", &component.TextString);
            UI::DrawColor4Control("Color", component.Color);
            UI::DrawFloatControl("Kerning", &component.Kerning, 0.025f);
            UI::DrawFloatControl("Line Spacing", &component.LineSpacing, 0.025f);
        });

        DrawComponent<CAnimPlayer>("CAnimPlayer", entity, [](auto& component)
        {
            std::string btnLabel = "None";
            if (component.AnimClip != 0)
            {
                if (AssetManager::IsAssetHandleValid(component.AnimClip))
                {
                    AssetType type = AssetManager::GetAssetType(component.AnimClip);
                    if (type == AssetType::AnimClip)
                    {
                        btnLabel = AssetManager::GetAsset<AnimClip>(component.AnimClip)->Name;
                    }
                    else if (type == AssetType::AnimMachine)
                    {
                        const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.AnimClip);
                        btnLabel = metadata.FilePath.filename().string();
                    }
                }
                else
                {
                    btnLabel = "Invalid";
                }
            }

            AssetHandle handle = component.AnimClip;
            AssetType outType = component.Type;
            if (UI::DrawAssetDragDropBox("Clip", btnLabel, &handle, { AssetType::AnimClip, AssetType::AnimMachine }, &outType))
            {
                component.AnimClip = handle;
                component.Type = outType;
            }
        });
    }

    void SceneHierarchyPanel::OnSavePrefab(SavePrefabEvent& e)
    {
        Entity rootEntity = m_Context->GetEntityByUUID(e.Root);
        std::filesystem::path path = e.Dir / (rootEntity.GetName() + ".nsprefab");
        SceneSerializer serializer(m_Context);
        serializer.SerializePrefabFrom(path, rootEntity.GetUUID());
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