#include "pch.h"
#include "RenderSystem.h"

#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Renderer/Renderer2D.h"

namespace Nous
{
    static Scene* s_Scene = nullptr;
    static float s_UICameraRatioW = 16.0f;
    static float s_UICameraRatioH = 9.0f;
    static float s_UICameraRatio = s_UICameraRatioW / s_UICameraRatioH;
    static EditorCamera s_UICamera = EditorCamera(30.0f, s_UICameraRatio, 0.1f, 1000.0f);

	void RenderSystem::Start(Scene* scene)
	{
        s_Scene = scene;
	}

	void RenderSystem::Update(Timestep dt, EditorCamera* camera)
	{
        NS_PROFILE_FUNCTION();

        if (!s_Scene)
            return;

        // 2D渲染
        Camera* mainCamera = camera;
        glm::mat4 cameraTransform;
        bool useEditorCamera = camera != nullptr;

        if (!useEditorCamera)
        {
            auto view = s_Scene->GetAllEntitiesWith<CTransform, CCamera>();
            for (auto ent : view)
            {
                auto [transform, camera] = view.get<CTransform, CCamera>(ent);

                if (!transform.Active)
                    continue;

                if (!camera.Primary)
                    continue;
                
                mainCamera = &camera.Camera;
                cameraTransform = transform.GetTransform();
                break;
            }
        }
        

        if (mainCamera)
        {
            if (useEditorCamera)
                Renderer2D::BeginScene(*camera);
            else
                Renderer2D::BeginScene(*mainCamera, cameraTransform);

            // Sprites
            {
                auto view = s_Scene->GetAllEntitiesWith<CTransform, CSpriteRenderer>();
                view.each([](auto ent, CTransform &transform, CSpriteRenderer &sprite)
                {
                    NS_PROFILE_SCOPE("Sprite Render");
                    if (transform.Active)
                    {
                        auto worldTransform = transform.ParentTransform * transform.GetTransform();
                        Renderer2D::DrawSprite(worldTransform, sprite, (int)ent);
                    }
                });
            }

            // Circles
            {
                auto view = s_Scene->GetAllEntitiesWith<CTransform, CCircleRenderer>();
                for (auto ent : view)
                {
                    auto [transform, circle] = view.get<CTransform, CCircleRenderer>(ent);

                    if (!transform.Active)
                        continue; 
                    
                    Renderer2D::DrawCircle(transform.ParentTransform * transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)ent);
                }

            }

            // Text
            {
                auto view = s_Scene->GetAllEntitiesWith<CTransform, CTextRenderer>();
                for (auto ent : view)
                {
                    auto [transform, text] = view.get<CTransform, CTextRenderer>(ent);

                    if (!transform.Active)
                        continue;

                    Renderer2D::DrawString(transform.ParentTransform * transform.GetTransform(), text.TextString, text, text.Color, (int)ent);
                }
            }

            auto view = s_Scene->GetAllEntitiesWith<CTransform, CUIButton>();
            for (auto ent : view)
            {
                auto [transform, btn] = view.get<CTransform, CUIButton>(ent);

                if (!transform.Active)
                    continue;

                float aspect = useEditorCamera ? camera->GetAspectRatio() : ((SceneCamera*)mainCamera)->GetAspectRatio();
                float offsetX = 0.0f;
                float offsetY = 0.0f;

                switch (btn.AnchorH)
                {
                case UIHorizontalAnchor::Left:  offsetX = -1.0f * aspect;   break;
                case UIHorizontalAnchor::Right: offsetX = 1.0f * aspect;    break;
                }

                switch (btn.AnchorV)
                {
                case UIVerticalAnchor::Bottom:  offsetY = -1.0f;            break;
                case UIVerticalAnchor::Top:     offsetY = 1.0f;             break;
                }

                // 可能通过新的shader，将部分矩阵运算交给gpu
                glm::mat4 uiTransform(1.0f);
                // ui 应该是固定在屏幕上，不受摄像机属性影响
                // 对抗viewproject矩阵
                if (!useEditorCamera)
                    uiTransform = glm::inverse(mainCamera->GetProjectionMatrix() * glm::inverse(cameraTransform));
                else
                    uiTransform = glm::inverse(camera->GetViewProjectionMatrix());
                
                uiTransform *= transform.ParentTransform 
                    * glm::scale(glm::mat4(1.0f), glm::vec3(btn.Size.x / aspect, btn.Size.y, 1.0f)) // 对抗摄像机长宽比
                    * glm::translate(glm::mat4(1.0f), glm::vec3(offsetX, offsetY, 0)) // 根据锚点进行偏移
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

	void RenderSystem::Stop()
	{
        s_Scene = nullptr;
	}
}