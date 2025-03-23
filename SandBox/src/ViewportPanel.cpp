#include "ViewportPanel.h"

#include "Nous/Core/Application.h"
#include "Nous/Scene/System/TransformSystem.h"
#include "Nous/Scene/System/UISystem.h"

#include "Nous/Event/AppEvent.h"

#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>

namespace Nous
{
    namespace Utils
    {
        static bool IsCameraAlignedWithAxis(const glm::quat& cameraOrientation, const glm::vec3& axis) {
            // 获取摄像机的前向量，即方向向量
            const glm::vec3 cameraForward = cameraOrientation * glm::vec3(0.0f, 0.0f, -1.0f);

            // 计算摄像机方向和指定轴的夹角
            const float dotProduct = glm::dot(glm::normalize(cameraForward), glm::normalize(axis));

            // 检查点积是否接近 1 或 -1（与轴平行或反向平行）
            return glm::abs(dotProduct) > 0.999f;
        }
    }

    ViewportPanel::ViewportPanel(const Ref<Framebuffer>& framebuffer)
    {
        SetFramebuffer(framebuffer);
    }

    void ViewportPanel::SetFramebuffer(const Ref<Framebuffer>& framebuffer)
    {
        m_Framebuffer = framebuffer;
        UISystem::SetFramebuffer(framebuffer);
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
        NS_PROFILE_FUNCTION();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        ImGui::Begin("Viewport");

        static bool isOnRunning = false;
        if (m_Context)
        {
            if (m_Context->IsRunning() && !isOnRunning)
                ImGui::SetWindowFocus();
            isOnRunning = m_Context->IsRunning();
        }

        const auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        const auto viewportOffset = ImGui::GetWindowPos(); // 包含标签栏
        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
        m_ViewportContentSize = m_ViewportBounds[1] - m_ViewportBounds[0];

        const ImVec2 mousePos_ = ImGui::GetMousePos();
        UISystem::SetViewport({ mousePos_.x, mousePos_.y }, m_ViewportBounds[0], m_ViewportContentSize);


        // TODO 修复焦点不在Viewport不能用快捷键的bug
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->SetBlockEvent(!m_ViewportHovered);

        const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        m_EditorCamera->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

        const uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
        ImGui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, { 0, 1 }, { 1, 0 });

        // 设置拖放目标
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_BROWSER_ITEM"))
            {
                AssetHandle handle = *(AssetHandle*)payload->Data;
                OpenSceneEvent event{ handle };
                AppEventEmitter::Emit(event);
            }
            ImGui::EndDragDropTarget();
        }


        ImGui::End();
        ImGui::PopStyleVar();
    }



    void ViewportPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowDropEvent>(NS_BIND_EVENT_FN(ViewportPanel::OnWindowDrop));
    }


    bool ViewportPanel::OnWindowDrop(WindowDropEvent& e)
    {
        for (auto& filepath : e.GetPaths())
        {
            NS_INFO("Drop file: {}", filepath.string());
        }
        return true;
    }

    void ViewportPanel::CheckHoveredEntity()
    {
        auto [mx, my] = ImGui::GetMousePos();
        mx -= GetMinBound().x;
        my -= GetMinBound().y;
        // 不包含标签栏
        auto viewportContentSize = GetContentSize();
        my = viewportContentSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportContentSize.x && mouseY < (int)viewportContentSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_Context.get());
        }
    }
}