#include "pch.h"
#include "RenderSystem.h"

#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Renderer/Renderer2D.h"

#include "Nous/Scene/System/TransformSystem.h"

namespace Nous
{
    static Scene* s_Scene = nullptr;
    static float s_UICameraRatioW = 16.0f;
    static float s_UICameraRatioH = 9.0f;
    static float s_UICameraRatio = s_UICameraRatioW / s_UICameraRatioH;
    static EditorCamera s_UICamera = EditorCamera(30.0f, s_UICameraRatio, 0.1f, 1000.0f);
    static float s_AspectStore = -1.0f;


	void RenderSystem::Start(Scene* scene)
	{
        s_Scene = scene;
	}

	void RenderSystem::Update(Timestep dt, EditorCamera* camera)
	{
        NS_PROFILE_FUNCTION();

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

                    Renderer2D::DrawString(transform.ParentTransform * transform.GetTransform(), text.Text, text, text.Color, (int)ent);
                }
            }

            Renderer2D::EndScene();

            // ÉèÖÃaspect
            const float aspect = useEditorCamera ? camera->GetAspectRatio() : ((SceneCamera*)mainCamera)->GetAspectRatio();
            if (aspect != s_AspectStore) {
                s_AspectStore = aspect;
                TransformSystem::SetSubtreeDirty(s_Scene);
            }
        }
	}

	void RenderSystem::Stop()
	{
        s_Scene = nullptr;
	}

    void RenderSystem::ClearAspectCache()
    {
        s_AspectStore = -1.0f;
    }

    float RenderSystem::GetAspectCache()
    {
        return s_AspectStore;
    }
}