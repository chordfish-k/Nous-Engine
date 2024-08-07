#include "pch.h"
#include "ViewportPanel.h"

#include "Nous/Core/Application.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Scene/Component.h"

#include "SceneHierarchyPanel.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace Nous {


    ViewportPanel::ViewportPanel(const Ref<Framebuffer>& framebuffer)
    {
        SetFramebuffer(framebuffer);
    }

    void ViewportPanel::SetFramebuffer(const Ref<Framebuffer>& framebuffer)
    {
        m_Framebuffer = framebuffer;
    }

    void ViewportPanel::SetContent(const Ref<Scene>& scene)
    {
        m_Context = scene;
    }

    void ViewportPanel::SetEditorCamera(const Ref<EditorCamera>& camera)
    {
        m_EditorCamera = camera;
    }

    void ViewportPanel::OnImGuiRender()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::Begin("Viewport");

        auto viewportOffset = ImGui::GetCursorPos(); // 包含标签栏

        // TODO 修复焦点不在Viewport不能用快捷键的bug
        bool flag = ImGui::IsAnyItemActive() ?
            (!ImGui::IsWindowFocused() || !ImGui::IsWindowHovered()) :
            (!ImGui::IsWindowFocused() && !ImGui::IsWindowHovered());
        Application::Get().GetImGuiLayer()->SetBlockEvent(flag);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        m_EditorCamera->SetViewportSize(m_ViewportSize.x,  m_ViewportSize.y);

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
        ImGui::Image((void*) textureID, {m_ViewportSize.x, m_ViewportSize.y}, {0, 1}, {1, 0});

        auto windowSize = ImGui::GetWindowSize();
        auto minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        ImVec2 maxBound = {minBound.x + windowSize.x, minBound.y + windowSize.y };
        m_ViewportBounds[0] = { minBound.x, minBound.y }; // 左上角
        m_ViewportBounds[1] = { maxBound.x, maxBound.y }; // 右下角
        m_ViewportContentSize = m_ViewportBounds[1] - m_ViewportBounds[0];

        // Gizmos
        Entity selectedEntity = m_Context->GetSelectedEntity();
        if (selectedEntity && m_GizmoType >= 0 && m_GizmoType <= 3)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float) ImGui::GetWindowWidth();
            float windowHeight = (float) ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

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

        ImGui::End();
        ImGui::PopStyleVar();
    }
}