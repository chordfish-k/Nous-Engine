#include "pch.h"
#include "Scene.h"

#include "Entity.h"
#include "Nous/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

namespace Nous {

    static void OnTransformConstruct()
    {

    }

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<CTransform>();
        auto& tag = entity.AddComponent<CTag>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::OnUpdate(Timestep dt)
    {
        // 渲染精灵图
        Camera* mainCamera = nullptr;
        glm::mat4* cameraTransform = nullptr;
        {
            auto view = m_Registry.view<CTransform, CCamera>();
            for (auto ent : view)
            {
                auto[transform, camera] = view.get<CTransform, CCamera>(ent);
                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = &transform.Transform;
                    break;
                }
            }
        }

        if (mainCamera && cameraTransform)
        {
            Renderer2D::BeginScene(*mainCamera, *cameraTransform);

            auto group = m_Registry.group<CTransform>(entt::get<CSpriteRenderer>);
            for (auto ent : group)
            {
                auto[transform, sprite] = group.get<CTransform, CSpriteRenderer>(ent);

                Renderer2D::DrawQuad(transform, sprite.Color);
            }

            Renderer2D::EndScene();
        }

    }
}