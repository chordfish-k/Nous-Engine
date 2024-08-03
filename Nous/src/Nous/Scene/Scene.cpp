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
//        struct MeshComponent
//        {
//            float Value;
//            MeshComponent() = default;
//        };
//
//        struct TransformComponent
//        {
//            glm::mat4 Transform;
//
//            TransformComponent() = default;
//            TransformComponent(const TransformComponent&) = default;
//            TransformComponent(const glm::mat4& transform)
//                : Transform(transform) {};
//
//            operator glm::mat4&() { return Transform; }
//            operator const glm::mat4&() const { return Transform; }
//        };
//
//        entt::entity entity = m_Registry.create();
//        m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));
//
//        m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
//
//        if (m_Registry.any_of<TransformComponent>(entity))
//            TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
//
//        // view:找出所有包含某个组件的实体
//        auto view = m_Registry.view<TransformComponent>();
//        for (auto ent : view)
//        {
//            TransformComponent& transform = m_Registry.get<TransformComponent>(ent);
//        }
//
//        auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
//        for (auto ent : group)
//        {
//            auto[transform, mesh] = group.get<TransformComponent, MeshComponent>(ent);
//        }
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
        //取出同时有多个组件的实体
        auto group = m_Registry.group<CTransform>(entt::get<CSpriteRenderer>);
        for (auto ent : group)
        {
            auto[transform, sprite] = group.get<CTransform, CSpriteRenderer>(ent);

            Renderer2D::DrawQuad(transform, sprite.Color);
        }
    }
}