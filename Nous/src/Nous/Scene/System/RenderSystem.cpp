#include "pch.h"
#include "RenderSystem.h"

#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Renderer/Renderer2D.h"

namespace Nous
{
    static Scene* s_Scene = nullptr;

	void RenderSystem::Start(Scene* scene)
	{
        s_Scene = scene;
	}

	void RenderSystem::Update(Timestep dt, EditorCamera* camera)
	{
        if (!s_Scene)
            return;

        // 2DäÖÈ¾
        Camera* mainCamera = camera;
        glm::mat4 cameraTransform;
        bool useEditorCamera = camera != nullptr;

        if (!useEditorCamera)
        {
            auto view = s_Scene->GetAllEntitiesWith<CTransform, CCamera>();
            for (auto ent : view)
            {
                auto [transform, camera] = view.get<CTransform, CCamera>(ent);

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
                for (auto ent : view)
                {
                    auto [transform, sprite] = view.get<CTransform, CSpriteRenderer>(ent);
                    auto worldTransform = transform.ParentTransform * transform.GetTransform();
                    Renderer2D::DrawSprite(worldTransform, sprite, (int)ent);
                }
            }

            // Circles
            {
                auto view = s_Scene->GetAllEntitiesWith<CTransform, CCircleRenderer>();
                for (auto ent : view)
                {
                    auto [transform, circle] = view.get<CTransform, CCircleRenderer>(ent);
                    Renderer2D::DrawCircle(transform.ParentTransform * transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)ent);
                }

            }

            // Text
            {
                auto view = s_Scene->GetAllEntitiesWith<CTransform, CTextRenderer>();
                for (auto ent : view)
                {
                    auto [transform, text] = view.get<CTransform, CTextRenderer>(ent);
                    Renderer2D::DrawString(transform.ParentTransform * transform.GetTransform(), text.TextString, text, text.Color, (int)ent);
                }
            }

            Renderer2D::EndScene();
        }
	}

	void RenderSystem::Stop()
	{
        s_Scene = nullptr;
	}
}