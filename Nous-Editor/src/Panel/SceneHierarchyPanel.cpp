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
#include "Nous/ImGui/UI.h"


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
        AppEventEmitter::AddObserver(this);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
    {
        m_Context = scene;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        NS_PROFILE_FUNCTION();

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

    void SceneHierarchyPanel::OnEditorEvent(AppEvent& e)
    {
        AppEventDispatcher dispatcher(e);
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

        void* id = (void*)(uint64_t)(uint32_t)entity;

        // Non-Active color
        {
            EUI::ScopedStyleColor color(ImGuiCol_Text, 
                transform.Active ? ImVec4{1, 1, 1, 1} : ImVec4{ 0.6, 0.6, 0.6, 1 });

            transform.Open = ImGui::TreeNodeEx(id, flags, "%s %s", tag.Tag.c_str(), transform.PrefabAsset ? "[Prefab]" : "");
        }

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
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool hasDeleteBtn = true)
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
            ImGui::PopStyleVar();

            if (hasDeleteBtn)
            {
                ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
                if (ImGui::Button("x", { lineHeight, lineHeight }))
                {
                    ImGui::OpenPopup("ComponentSettings");
                }
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
        {
            auto& tr = entity.GetTransform();
            if (ImGui::Checkbox("##Active", &tr.Active))
            {
                TransformSystem::SetSubtreeActive(m_Context.get(), entity, tr.Active);
            }
        }

        ImGui::SameLine();

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

            ImGui::Separator();

            DisplayAddComponentEntry<CUIAnchor>("CUIAnchor");
            DisplayAddComponentEntry<CUIEventBubble>("CUIEventBubble");
            DisplayAddComponentEntry<CUIButton>("CUIButton");
            DisplayAddComponentEntry<CUIText>("CUIText");
            DisplayAddComponentEntry<CUIImage>("CUIImage");

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

            if (EUI::DrawVec3Control("Position", component.Translation)) changed = true;
            glm::vec3 rotation = glm::degrees(component.Rotation);
            if (EUI::DrawVec3Control("Rotation", rotation)) changed = true;
            component.Rotation = glm::radians(rotation);
            if (EUI::DrawVec3Control("Scale", component.Scale, 1.0f)) changed = true;

            if (changed)
                TransformSystem::SetSubtreeDirty(m_Context.get(), entity);
        }, false);


        DrawComponent<CUIAnchor>("CUIAnchor", entity, [&](CUIAnchor& component)
        {
            const char* anchorHStrings[] = { "Center", "Left", "Right" };
            const char* anchorVStrings[] = { "Center", "Top", "Bottom" };
            uint32_t anchorHIndex = (int)component.AnchorH;
            uint32_t anchorVIndex = (int)component.AnchorV;

            bool changed = false;
            if (EUI::DrawCombo("Anchor H", anchorHStrings, &anchorHIndex, 3))
            {
                component.AnchorH = (UIHorizontalAnchor)anchorHIndex;
                changed = true;
            }

            if (EUI::DrawCombo("Anchor V", anchorVStrings, &anchorVIndex, 3))
            {
                component.AnchorV = (UIVerticalAnchor)anchorVIndex;
                changed = true;
            }
            if (changed)
                TransformSystem::SetSubtreeDirty(m_Context.get(), entity);
        });
        

        DrawComponent<CUIEventBubble>("CUIEventBubble", entity, [&](CUIEventBubble& component)
        {
            EUI::DrawCheckbox("Bubble Up", &component.EventBubbleUp);
        });


        DrawComponent<CCamera>("CCamera", entity, [](auto& component){
            auto& camera = component.Camera;
            EUI::DrawCheckbox("Primary", &component.Primary);

            const char* projectionTypeStrings[] = {"Perspective", "Orthographic"};
            uint32_t currentIndex = (int)camera.GetProjectionType();
            if (EUI::DrawCombo("Projection", projectionTypeStrings, &currentIndex, 2))
            {
                camera.SetProjectionType((SceneCamera::ProjectionType)currentIndex);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float FOV = glm::degrees(camera.GetPerspFOV());
                if (EUI::DrawFloatControl("FOV", &FOV))
                    camera.SetPerspFOV(glm::radians(FOV));

                float perspNear = camera.GetPerspNearClip();
                if (EUI::DrawFloatControl("Near", &perspNear))
                    camera.SetPerspNearClip(perspNear);

                float perspFar = camera.GetPerspFarClip();
                if (EUI::DrawFloatControl("Far", &perspFar))
                    camera.SetPerspFarClip(perspFar);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthoSize();
                if (EUI::DrawFloatControl("Size", &orthoSize))
                    camera.SetOrthoSize(orthoSize);

                float orthoNear = camera.GetOrthoNearClip();
                if (EUI::DrawFloatControl("Near", &orthoNear))
                    camera.SetOrthoNearClip(orthoNear);

                float orthoFar = camera.GetOrthoFarClip();
                if (EUI::DrawFloatControl("Far", &orthoFar))
                    camera.SetOrthoFarClip(orthoFar);

                EUI::DrawCheckbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
            }
        });

        DrawComponent<CMonoScript>("CMonoScript", entity, [entity, scene = m_Context](auto& component) mutable // mutable:允许修改捕获的变量
        {
            bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

            static char buffer[64];
            strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

            // Class 脚本类名
            {
                EUI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);
                if (EUI::DrawInputText("Class", buffer, sizeof(buffer)))
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
                        if (field.Type == ScriptFieldType::Int)
                        {
                            auto data = scriptInstance->GetFieldValue<int>(name);
                            if (EUI::DrawIntControl(name, &data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Float)
                        {
                            auto data = scriptInstance->GetFieldValue<float>(name);
                            if (EUI::DrawFloatControl(name, &data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Vector2)
                        {
                            auto data = scriptInstance->GetFieldValue<glm::vec2>(name);
                            if (EUI::DrawVec2Control(name, data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            auto data = scriptInstance->GetFieldValue<bool>(name);
                            if (EUI::DrawCheckbox(name, &data))
                            {
                                scriptInstance->SetFieldValue(name, data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Prefab)
                        {
                            auto data = scriptInstance->GetFieldValue<AssetHandleWrapper>(name);
                            AssetHandle handle = data.Handle;
                            if (EUI::DrawAssetDragDropBox(name, AssetManager::GetAssetFileName(handle), &handle, AssetType::Prefab))
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
                        if (field.Type == ScriptFieldType::Int)
                        {
                            auto data = scriptField.GetValue<int>();
                            if (EUI::DrawIntControl(name, &data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Float)
                        {
                            auto data = scriptField.GetValue<float>();
                            if (EUI::DrawFloatControl(name, &data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Vector2)
                        {
                            auto data = scriptField.GetValue<glm::vec2>();
                            if (EUI::DrawVec2Control(name, data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            auto data = scriptField.GetValue<bool>();
                            if (EUI::DrawCheckbox(name, &data))
                                scriptField.SetValue(data);
                        }
                        else if (field.Type == ScriptFieldType::Prefab)
                        {
                            auto data = scriptField.GetValue<AssetHandleWrapper>();
                            AssetHandle handle = data.Handle;
                            if (EUI::DrawAssetDragDropBox(name, AssetManager::GetAssetFileName(handle), &handle, AssetType::Prefab))
                            {
                                data.Handle = handle;
                                scriptField.SetValue(data);
                            }
                        }
                    }
                    else
                    {
                        if (field.Type == ScriptFieldType::Int)
                        {
                            int data = 0;
                            if (EUI::DrawIntControl(name, &data))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                scriptField.SetValue(data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Float)
                        {
                            float data = 0.0f;
                            if (EUI::DrawFloatControl(name, &data))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                scriptField.SetValue(data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Vector2)
                        {
                            glm::vec2 data(0.0f);
                            if (EUI::DrawVec2Control(name, data))
                            {
                                ScriptFieldInstance& scriptField = entityFields[name];

                                scriptField.Field = field;
                                scriptField.SetValue(data);
                            }
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            bool data = false;
                            if (EUI::DrawCheckbox(name, &data))
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
                            if (EUI::DrawAssetDragDropBox(name, AssetManager::GetAssetFileName(handle), &handle, AssetType::Prefab))
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
            AssetHandle handle = component.Texture;
            if (EUI::DrawAssetDragDropBox("Texture", AssetManager::GetAssetFileName(component.Texture), &handle, AssetType::Texture2D))
                component.Texture = handle;
            EUI::DrawColor4Control("Color", component.Color);
            EUI::DrawFloatControl("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
            EUI::DrawIntControl("Sheet Width", &component.SheetWidth, 0);
            EUI::DrawIntControl("Sheet Height", &component.SheetHeight, 0);
            EUI::DrawIntControl("Index", &component.Index, 0);
        });

        DrawComponent<CCircleRenderer>("CCircleRenderer", entity, [](auto& component)
        {
            EUI::DrawColor4Control("Color", component.Color);
            EUI::DrawFloatControl("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
        });

        DrawComponent<CRigidbody2D>("CRigidbody2D", entity, [](auto& component)
        {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            uint32_t currentIndex = (int) component.Type;
            if (EUI::DrawCombo("Body Type", bodyTypeStrings, &currentIndex, 3))
            {
                component.Type = (CRigidbody2D::BodyType)currentIndex;
            }

            EUI::DrawCheckbox("Fixed Rotation", &component.FixedRotation);
        });

        DrawComponent<CBoxCollider2D>("CBoxCollider2D", entity, [](auto& component)
        {
            EUI::DrawVec2Control("Offset", component.Offset);
            EUI::DrawVec2Control("Size", component.Size);
            EUI::DrawFloatControl("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        DrawComponent<CCircleCollider2D>("CCircleCollider2D", entity, [](auto& component)
        {
            EUI::DrawVec2Control("Offset", component.Offset);
            EUI::DrawFloatControl("Radius", &component.Radius);
            EUI::DrawFloatControl("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            EUI::DrawFloatControl("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        DrawComponent<CTextRenderer>("CTextRenderer", entity, [](auto& component)
        {
            EUI::DrawInputTextMultiline("Text", &component.Text);
            EUI::DrawColor4Control("Color", component.Color);
            EUI::DrawFloatControl("Kerning", &component.Kerning, 0.025f);
            EUI::DrawFloatControl("Line Spacing", &component.LineSpacing, 0.025f);
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
            if (EUI::DrawAssetDragDropBox("Clip", btnLabel, &handle, { AssetType::AnimClip, AssetType::AnimMachine }, &outType))
            {
                component.AnimClip = handle;
                component.Type = outType;
            }
        });


        // UI
        DrawComponent<CUIButton>("CUIButton", entity, [](CUIButton& component)
        {
            EUI::DrawColor4Control("Idle Color", component.IdleColor);
            EUI::DrawColor4Control("Hover Color", component.HoverColor);
            EUI::DrawColor4Control("Active Color", component.ActiveColor);

            EUI::DrawVec2Control("Size", component.Size);
            AssetHandle handle = component.Image;
            if (EUI::DrawAssetDragDropBox("Image", AssetManager::GetAssetFileName(handle), &handle, AssetType::Texture2D))
                component.Image = handle;

            EUI::DrawIntControl("Sheet Width", &component.SheetWidth, 0);
            EUI::DrawIntControl("Sheet Height", &component.SheetHeight, 0);
            EUI::DrawIntControl("Index", &component.Index, 0);

            static char bufferEntity[64];
            static char bufferFunction[64];
            strcpy_s(bufferEntity, sizeof(bufferEntity), component.InvokeEntity.c_str());
            strcpy_s(bufferFunction, sizeof(bufferFunction), component.InvokeFunction.c_str());

            // 要触发的实体名
            if (EUI::DrawInputText("Invoke Entity", bufferEntity, sizeof(bufferEntity)))
                component.InvokeEntity = bufferEntity;

            // 要触发的实体脚本内方法名
            if (EUI::DrawInputText("Invoke Function", bufferFunction, sizeof(bufferFunction)))
                component.InvokeFunction = bufferFunction;
        });

        DrawComponent<CUIText>("CUIText", entity, [](CUIText& component)
        {
            EUI::DrawInputTextMultiline("Text", &component.Text);
            EUI::DrawColor4Control("Color", component.Color);
            EUI::DrawFloatControl("Size", &component.Size);
            EUI::DrawFloatControl("Kerning", &component.Kerning, 0.025f);
            EUI::DrawFloatControl("Line Spacing", &component.LineSpacing, 0.025f);
        });

        DrawComponent<CUIImage>("CUIImage", entity, [](CUIImage& component)
        {
            AssetHandle handle = component.Image;
            if (EUI::DrawAssetDragDropBox("Texture", AssetManager::GetAssetFileName(component.Image), &handle, AssetType::Texture2D))
                component.Image = handle;
            EUI::DrawColor4Control("Color", component.Color);
            EUI::DrawVec2Control("Size", component.Size);
            EUI::DrawIntControl("Sheet Width", &component.SheetWidth, 0);
            EUI::DrawIntControl("Sheet Height", &component.SheetHeight, 0);
            EUI::DrawIntControl("Index", &component.Index, 0);
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