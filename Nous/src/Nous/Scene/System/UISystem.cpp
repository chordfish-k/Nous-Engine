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
            if (e.HasComponent<CUIEventBubble>())
            {
                auto& ui = e.GetComponent<CUIEventBubble>();

                ui.IsHovering = true;

                if (Input::IsMouseButtonPressed(Mouse::Button0))
                {
                    if (!ui.IsPressing)
                    {
                        ui.IsPressing = true;

                        // Invoke
                        EventEmit(e);
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

    void UISystem::EventEmit(Entity entityCurr)
    {
        while (bool bubbleUp = OnClickEvent(entityCurr))
        {
            auto& tr = entityCurr.GetTransform();
            if (tr.Parent)
            {
                entityCurr = s_Scene->GetEntityByUUID(tr.Parent);
            }
            else break;
        }
    }

    bool UISystem::OnClickEvent(Entity entity)
    {
        if (!entity.HasComponent<CUIEventBubble>())
            return false;
        auto& uiEventHolder = entity.GetComponent<CUIEventBubble>();

        if (!entity.HasComponent<CUIButton>())
            return uiEventHolder.EventBubbleUp;
        auto& uiBtn = entity.GetComponent<CUIButton>();

        Entity invokeEntity = s_Scene->GetEntityByName(uiBtn.InvokeEntity);
        if (invokeEntity)
        {
            ScriptEngine::InvokeInstanceMethod(invokeEntity, uiBtn.InvokeFunction);
        }
        else
        {
            NS_CORE_ERROR("找不到实体 \"{}\"", uiBtn.InvokeEntity);
        }
        return uiEventHolder.EventBubbleUp;
    }

    void UIRenderSystem::Update(Timestep dt)
    {
        if (!s_Scene)
            return;

        Renderer2D::BeginUIScene();

        // Button
        s_Scene->GetAllEntitiesWith<CTransform, CUIButton>()
            .each([&](entt::entity ent, CTransform& transform, CUIButton& btn)
        {
            if (!transform.Active)
                return;

            Entity entity{ ent, s_Scene };

            // 可能通过新的shader，将部分矩阵运算交给gpu
            // ui 应该是固定在屏幕上，不受摄像机属性影响
            glm::mat4 uiTransform = glm::scale(glm::mat4(1.0f), { 1.0f / RenderSystem::GetAspectCache(), 1.0f, 1.0f })
                * transform.ParentTransform
                * transform.GetTransform()
                * glm::scale(glm::mat4(1.0f), glm::vec3(btn.Size.x, btn.Size.y, 1.0f))
                ;

            glm::vec4 color = btn.IdleColor;
            if (entity.HasComponent<CUIEventBubble>())
            {
                auto& eventHolder = entity.GetComponent<CUIEventBubble>();
                if (eventHolder.IsHovering) {
                    color = btn.HoverColor;
                    eventHolder.IsHovering = false;
                }
                if (eventHolder.IsPressing) {
                    color = btn.ActiveColor;
                }
            }
            Renderer2D::DrawQuad(uiTransform, color, (int)ent);
        });


        s_Scene->GetAllEntitiesWith<CTransform, CUIText>()
            .each([&](entt::entity ent, CTransform& transform, CUIText& text)
        {
            if (!transform.Active)
                return;

            Entity entity{ ent, s_Scene };

            glm::vec2 size = Renderer2D::GetDrawStringSize(text.Text, text.FontAsset, {});

            glm::mat4 uiTransform =
                glm::scale(glm::mat4(1.0f), { 1.0f / RenderSystem::GetAspectCache(), 1.0f, 1.0f })
                * transform.ParentTransform
                * transform.GetTransform()
                * glm::scale(glm::mat4(1.0f), glm::vec3(text.Size))
                * glm::translate(glm::mat4(1.0f), { -size.x * 0.5f, size.y * 0.5f, 0 })
                ;

            Renderer2D::DrawString(uiTransform, text.Text, text.FontAsset,
                Renderer2D::TextParams{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
                (int)ent);
        });

        Renderer2D::EndScene();
    }
}