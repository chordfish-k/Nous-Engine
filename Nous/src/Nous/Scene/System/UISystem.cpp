#include "pch.h"
#include "UISystem.h"

#include "Nous/Core/Application.h"
#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Core/MouseCodes.h"

#include "Nous/Script/ScriptEngine.h"

#include "Nous/Renderer/Renderer2D.h"
#include "Nous/Scene/System/RenderSystem.h"

#include <GLFW/glfw3.h>

namespace Nous
{
    static Scene* s_Scene = nullptr;
    static glm::vec2 s_mPos = glm::vec2(0.0f);
    static glm::vec2 s_vPos = glm::vec2(0.0f);
    static glm::vec2 s_vSize = glm::vec2(0.0f);

    static Ref<Framebuffer> s_fb;

    namespace Utils 
    {
        static Entity CheckHoveredEntity()
        {
            if (!s_Scene || !s_fb)
                return Entity();

            const glm::vec2 mouse = { s_mPos.x - s_vPos.x, s_vSize.y - (s_mPos.y - s_vPos.y) };
            const int mouseX = (int)mouse.x;
            const int mouseY = (int)mouse.y;
            if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)s_vSize.x && mouseY < (int)s_vSize.y)
            {
                int pixelData = s_fb->ReadPixel(1, mouseX, mouseY);
                return pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, s_Scene);
            }

            return Entity();
        }
    }

    void UISystem::Start(Scene* scene)
    {
        s_Scene = scene;
    }

    void UISystem::SetViewport(const glm::vec2& mousePos, const glm::vec2& viewpostLeftTop, const glm::vec2& viewpostSize)
    {
        s_mPos = mousePos;
        s_vPos = viewpostLeftTop;
        s_vSize = viewpostSize;
    }

    void UISystem::SetFramebuffer(Ref<Framebuffer> framebuffer)
    {
        s_fb = framebuffer;
    }

    void UISystem::Update(Timestep dt, const glm::vec2& viewpostLeftTop, const glm::vec2& viewpostSize)
    {
        if (!s_Scene || !s_fb)
            return;

        if (Entity e = Utils::CheckHoveredEntity())
        {
            if (e.HasComponent<CUIButton>())
            {
                auto& ui = e.GetComponent<CUIButton>();

                ui.IsHovering = true;

                if (Input::IsMouseButtonPressed(Mouse::Button0))
                {
                    if (!ui.IsPressing)
                    {
                        ui.IsPressing = true;

                        // Invoke
                        Entity invokeEntity = s_Scene->GetEntityByName(ui.InvokeEntity);
                        if (invokeEntity)
                        {
                            ScriptEngine::InvokeInstanceMethod(invokeEntity, ui.InvokeFunction);
                        }
                        else
                        {
                            NS_CORE_ERROR("找不到实体 \"{}\"", ui.InvokeEntity);
                        }
                    }
                }
                else 
                {
                    ui.IsPressing = false;
                }
            }

            
        }
    }

    void UISystem::Stop()
    {
        s_Scene = nullptr;
    }

    void UIRenderSystem::Update(Timestep dt)
    {
        if (!s_Scene)
            return;

        Renderer2D::BeginUIScene();

        auto view = s_Scene->GetAllEntitiesWith<CTransform, CUIButton>();
        for (auto ent : view)
        {
            auto [transform, btn] = view.get<CTransform, CUIButton>(ent);

            if (!transform.Active)
                continue;

            Entity entity{ ent, s_Scene };

            // 可能通过新的shader，将部分矩阵运算交给gpu
            // ui 应该是固定在屏幕上，不受摄像机属性影响
            glm::mat4 uiTransform = glm::scale(glm::mat4(1.0f), { 1.0f / RenderSystem::GetAspectCache(), 1.0f, 1.0f })
                * transform.ParentTransform
                * transform.GetTransform();

            glm::vec4 color = btn.IdleColor;
            if (btn.IsHovering) {
                color = btn.HoverColor;
                btn.IsHovering = false;
            }
            if (btn.IsPressing) {
                color = btn.ActiveColor;
            }
            Renderer2D::DrawQuad(uiTransform, color, (int)ent);
        }

        Renderer2D::EndScene();
    }
}