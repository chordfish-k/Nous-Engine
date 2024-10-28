#include "ViewportPanel.h"

#include "Nous/Core/Application.h"

#include "Panel/SceneHierarchyPanel.h"
#include "Event/EditorEvent.h"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Nous {
    extern const std::filesystem::path g_AssetPath;

    ViewportPanel::ViewportPanel(const Ref<Framebuffer>& framebuffer)
    {
        SetFramebuffer(framebuffer);
    }

    void ViewportPanel::SetFramebuffer(const Ref<Framebuffer>& framebuffer)
    {
        m_Framebuffer = framebuffer;
    }

    void ViewportPanel::SetContext(const Ref<Scene>& scene)
    {
        m_Context = scene;
        m_HoveredEntity = {};
    }

    void ViewportPanel::SetEditorCamera(EditorCamera* camera)
    {
        m_EditorCamera = camera;
    }

    void ViewportPanel::OnImGuiRender()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::Begin("Viewport");

        static bool isOnRunning = false;
        if (m_Context)
        {
            if (m_Context->IsRunning() && !isOnRunning)
                ImGui::SetWindowFocus();
            isOnRunning = m_Context->IsRunning();
        }

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos(); // 包含标签栏
        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
        m_ViewportContentSize = m_ViewportBounds[1] - m_ViewportBounds[0];

        // TODO 修复焦点不在Viewport不能用快捷键的bug
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->SetBlockEvent(!m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        m_EditorCamera->SetViewportSize(m_ViewportSize.x,  m_ViewportSize.y);

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
        ImGui::Image((void*) textureID, {m_ViewportSize.x, m_ViewportSize.y}, {0, 1}, {1, 0});

        // 设置拖放目标
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                // TODO 检测文件类型为.scn
                OpenSceneEvent event {std::filesystem::path(g_AssetPath) / path};
                EditorEventRepeater::Emit(event);
            }
            ImGui::EndDragDropTarget();
        }

        // Gizmos
        if (m_Context)
        {
            Entity selectedEntity = m_Context->GetSelectedEntity();
            if (selectedEntity && m_GizmoType >= 0 && m_GizmoType <= 3)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
                                  m_ViewportBounds[1].x - m_ViewportBounds[0].x,
                                  m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // TODO 切换不同的摄像机
                // Runtime camera
//            auto cameraEntity = m_Context->GetPrimaryCameraEntity();
//            const auto& camera = cameraEntity.GetComponent<CCamera>();
//            auto cameraProjection = camera.Camera.GetProjectionMatrix();
//            auto cameraView = glm::inverse(cameraEntity.GetComponent<CTransform>().GetTransform());

                // Editor camera
                const glm::mat4& cameraProjection = m_EditorCamera->GetProjectionMatrix();
                glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

                // Entity Transform
                auto& tc = selectedEntity.GetComponent<CTransform>();
                auto transform = tc.GetTransform();

                // Snapping 吸附
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                float snapValue = 0.5f; // 咬合位移/缩放到0.5大小
                // 将旋转角度吸附到45度
                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                float snapValues[3] = {snapValue, snapValue, snapValue};

                if (m_ShowGizmo)
                    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                                         (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                                         nullptr, snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    glm::mat4 tr = transform;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(tr), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

                    if (m_GizmoType == ImGuizmo::OPERATION::TRANSLATE)
                        tc.Translation = translation;
                    if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    {
                        // 解矩阵的旋转值不靠谱，手动吸附一下
                        if (snap)
                        {
                            const float snapV = snapValue * 1.00001f; // 防止万向节锁，乘一个近似1的小数
                            rotation = glm::round(rotation / snapV) * snapV; // 吸附
                        }
                        rotation = glm::mod(glm::mod(rotation, 360.0f) + 360.0f, 360.0f) ; // 锁定在0 ~ 360之间

                        glm::vec3 deltaRotation = glm::radians(rotation) - tc.Rotation;
                        tc.Rotation += deltaRotation;
                    }
                    if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
                        tc.Scale = scale;
                }
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }



    void ViewportPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(NS_BIND_EVENT_FN(ViewportPanel::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(NS_BIND_EVENT_FN(ViewportPanel::OnMouseButtonPressed));

    }

    bool ViewportPanel::OnKeyPressed(KeyPressedEvent& e)
    {
        // 快捷键
        if (e.IsRepeat())
            return false;

        switch (e.GetKeyCode())
        {
            // Gizmos
            case Key::Q:
            {
                if (!ImGuizmo::IsUsing() && m_ShowGizmo)
                    m_GizmoType = -1;
                break;
            }
            case Key::W:
            {
                if (!ImGuizmo::IsUsing() && m_ShowGizmo)
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case Key::E:
            {
                if (!ImGuizmo::IsUsing() && m_ShowGizmo)
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case Key::R:
            {
                if (!ImGuizmo::IsUsing() && m_ShowGizmo)
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
            default:
                return false;
        }
        return false;
    }

    bool ViewportPanel::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (IsHovered() && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
            {
                m_Context->SetSelectedEntity(m_HoveredEntity);
                if (m_GizmoType == -1)
                    m_GizmoType = 0;
                m_ShowGizmo = (bool) m_HoveredEntity;
            }
        }
        return false;
    }

    void ViewportPanel::CheckAndResize()
    {
        if (!m_Context || ! m_Framebuffer) return;

        auto spec = m_Framebuffer->GetSpecification();
        auto viewportSize = GetSize();
        if (viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
            (spec.Width != (uint32_t) viewportSize.x ||
             spec.Height != (uint32_t) viewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t) viewportSize.x, (uint32_t) viewportSize.y);
            m_Context->OnViewportResize((uint32_t) viewportSize.x, (uint32_t) viewportSize.y);
        }
    }

    void ViewportPanel::CheckHoveredEntity()
    {
        auto [mx, my] = ImGui::GetMousePos();
        mx -= GetMinBound().x;
        my -= GetMinBound().y;
        // 不包含标签栏
        auto viewportContentSize = GetContentSize();
        my = viewportContentSize.y - my;
        int mouseX = (int) mx;
        int mouseY = (int) my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportContentSize.x && mouseY < (int)viewportContentSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_Context.get());
        }
    }
}