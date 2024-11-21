#include "pch.h"
#include "UISystem.h"

#include "Nous/Core/Application.h"
#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Core/MouseCodes.h"

#include "Nous/Script/ScriptEngine.h"

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
        if (!s_Scene || ! s_fb)
            return;

        if (Entity e = Utils::CheckHoveredEntity())
        {
            if (e.HasComponent<CUIButton>())
            {
                auto& btn = e.GetComponent<CUIButton>();
                btn.IsHovering = true;

                if (Input::IsMouseButtonPressed(Mouse::Button0))
                {
                    if (!btn.IsPressing)
                    {
                        btn.IsPressing = true;

                        // Invoke
                        Entity invokeEntity = s_Scene->GetEntityByName(btn.InvokeEntity);
                        if (invokeEntity)
                        {
                            ScriptEngine::InvokeInstanceMethod(invokeEntity, btn.InvokeFunction);
                        }
                        else
                        {
                            NS_CORE_ERROR("找不到实体 {}", btn.InvokeEntity);
                        }
                    }
                }
                else 
                {
                    btn.IsPressing = false;
                }
            }
        }
    }

    void UISystem::Stop()
    {
        s_Scene = nullptr;
    }
}