#include "pch.h"
#include "Scene.h"

#include "Component.h"
#include "Nous/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

#include "Entity.h"

namespace Nous {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        // 执行脚本销毁
        {
            m_Registry.view<CNativeScript>().each([=](auto ent, auto& script) {
                if (script.Instance)
                {
                    script.Instance->OnDestroy();
                    script.DestroyScript(&script);
                }
            });
        }
    }

    Entity Scene::CreateEntity(const std::string& name, const glm::vec3& position)
    {
        Entity entity = {m_Registry.create(), this};
        auto& transform = entity.AddComponent<CTransform>();
        transform.Translation.x = position.x;
        transform.Translation.y = position.y;
        transform.Translation.z = position.z;

        auto& tag = entity.AddComponent<CTag>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdateEditor(Timestep dt, EditorCamera& camera)
    {
        Renderer2D::BeginScene(camera);

        auto group = m_Registry.group<CTransform>(entt::get<CSpriteRenderer>);
        for (auto ent: group)
        {
            auto [transform, sprite] = group.get<CTransform, CSpriteRenderer>(ent);

            Renderer2D::DrawSprite(transform, sprite, (int)ent);
        }

        Renderer2D::EndScene();
    }

    void Scene::OnUpdateRuntime(Timestep dt)
    {
        // 执行脚本更新
        {
            m_Registry.view<CNativeScript>().each([=](auto ent, auto& script) {
                // 没有脚本实例就先创建
                if (!script.Instance)
                {
                   script.Instance = script.InitScript();
                   script.Instance->m_Entity = Entity{ent, this};

                   script.Instance->OnCreate();
                }

                script.Instance->OnUpdate(dt);
            });
        }

        // 2D渲染
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = m_Registry.view<CTransform, CCamera>();
            for (auto ent: view)
            {
                auto [transform, camera] = view.get<CTransform, CCamera>(ent);

                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    break;
                }
            }
        }

        if (mainCamera)
        {
            Renderer2D::BeginScene(*mainCamera, cameraTransform);

            auto group = m_Registry.group<CTransform>(entt::get<CSpriteRenderer>);
            for (auto ent: group)
            {
                auto [transform, sprite] = group.get<CTransform, CSpriteRenderer>(ent);

                Renderer2D::DrawSprite(transform, sprite, (int)ent);
            }

            Renderer2D::EndScene();
        }

    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // 重设不固定高宽比的摄像机
        auto view = m_Registry.view<CCamera>();
        for (auto ent: view)
        {
            auto& cameraComponent = view.get<CCamera>(ent);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }

    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CCamera>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CCamera>(entity);
            if (camera.Primary)
                return Entity{entity, this};
        }
        return {};
    }

    Entity Scene::GetSelectedEntity()
    {
        return Entity{m_SelectedEntityID, this};
    }

    void Scene::SetSelectedEntity(Entity entity)
    {
        m_SelectedEntityID = entity;
    }

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(false);
    }

    template<>
    void Scene::OnComponentAdded<CTransform>(Entity entity, CTransform& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CCamera>(Entity entity, CCamera& component)
    {
        if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
            component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }

    template<>
    void Scene::OnComponentAdded<CTag>(Entity entity, CTag& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CSpriteRenderer>(Entity entity, CSpriteRenderer& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CNativeScript>(Entity entity, CNativeScript& component)
    {
    }
}