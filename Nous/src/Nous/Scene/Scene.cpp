#include "pch.h"
#include "Scene.h"

#include "Nous/Scene/Component.h"
#include "Nous/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

#include "Entity.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

namespace Nous {

    static b2BodyType Rigidbody2DTypeToBox2DBody(CRigidbody2D::BodyType bodyType)
    {
        switch (bodyType)
        {
            case CRigidbody2D::BodyType::Static:    return b2_staticBody;
            case CRigidbody2D::BodyType::Dynamic:   return b2_dynamicBody;
            case CRigidbody2D::BodyType::Kinematic: return b2_kinematicBody;
        }

        NS_CORE_ASSERT(false, "未知的 body type");
        return b2_staticBody;
    }

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

    void Scene::OnRuntimeStart()
    {
        m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
        auto view = m_Registry.view<CRigidbody2D>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            auto& transform = entity.GetComponent<CTransform>();
            auto& rb2d = entity.GetComponent<CRigidbody2D>();

            b2BodyDef bodyDef;
            bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;

            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            rb2d.RuntimeBody = body;

            if (entity.HasComponent<CBoxCollider2D>())
            {
                auto& bc2d = entity.GetComponent<CBoxCollider2D>();

                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = bc2d.Density;
                fixtureDef.friction = bc2d.Friction;
                fixtureDef.restitution = bc2d.Restitution;
                fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }
        }
    }

    void Scene::OnRuntimeStop()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
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

        // 物理更新
        {
            // 控制物理模拟的迭代次数
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

            // 从Box2D中取出transform数据
            auto view = m_Registry.view<CRigidbody2D>();
            for (auto e : view)
            {
                Entity entity = { e, this };
                auto& transform = entity.GetComponent<CTransform>();
                auto& rb2d = entity.GetComponent<CRigidbody2D>();

                b2Body* body = (b2Body*) rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = body->GetAngle();
            }
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

    template<>
    void Scene::OnComponentAdded<CRigidbody2D>(Entity entity, CRigidbody2D& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CBoxCollider2D>(Entity entity, CBoxCollider2D& component)
    {
    }
}