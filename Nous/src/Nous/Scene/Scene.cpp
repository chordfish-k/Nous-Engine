#include "pch.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"

#include "Nous/Scene/Component.h"
#include "Nous/Renderer/Renderer2D.h"
#include "Nous/Script/ScriptEngine.h"
#include "Nous/Physics/Physics2D.h"
#include "Nous/Asset/AssetManager.h"
#include "Nous/Anim/AnimMachine.h"

#include "Nous/Scene/System/RenderSystem.h"
#include "Nous/Scene/System/PhysicsSystem.h"
#include "Nous/Scene/System/ScriptSystem.h"
#include "Nous/Scene/System/AnimSystem.h"

#include <glm/glm.hpp>

namespace Nous {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    template<typename... Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        // C++17的参数包展开语法 -> (func(), ...) 对可变参数模板的每个类型都执行操作
        ([&]()
        {
            auto view = src.view<Component>();
            for (auto srcEntity : view)
            {
                entt::entity dstEntity = enttMap.at(src.get<CUuid>(srcEntity).ID);

                auto& srcComponent = src.get<Component>(srcEntity);
                dst.emplace_or_replace<Component>(dstEntity, srcComponent);
            }
        }(), ...);
    }

    template<typename... Component>
    static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        CopyComponent<Component...>(dst, src, enttMap);
    }

    template<typename... Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        ([&]()
        {
            if (src.HasComponent<Component>())
                dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
        }(), ...);
    }

    template<typename... Component>
    static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        CopyComponentIfExists<Component...>(dst, src);
    }

    Ref <Scene> Scene::Copy(Ref <Scene> other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();

        newScene->Handle = other->Handle;

        newScene->m_ViewportHeight = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        std::unordered_map<UUID, entt::entity> enttMap;

        auto& srcSceneRegistry = other->m_Registry;
        auto& dstSceneRegistry = newScene->m_Registry;
        auto idView = srcSceneRegistry.view<CUuid>();
        auto selected = Entity{ other->m_SelectedEntityID, other.get() };

        for (auto e : idView)
        {
            // 复制 Entity 到新场景
            UUID uuid = srcSceneRegistry.get<CUuid>(e).ID;
            const auto& name = srcSceneRegistry.get<CTag>(e).Tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;

            if (selected && e == selected)
                newScene->m_SelectedEntityID = (entt::entity)newEntity;
        }

        // 复制 Component 给新 Registry
        CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& name, const glm::vec3& position)
    {
        return CreateEntityWithUUID(UUID(), name, position);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const glm::vec3& position)
    {
        Entity entity = {m_Registry.create(), this};
        entity.AddComponent<CUuid>(uuid);

        auto& transform = entity.AddComponent<CTransform>();
        transform.Translation.x = position.x;
        transform.Translation.y = position.y;
        transform.Translation.z = position.z;

        auto& tag = entity.AddComponent<CTag>();
        tag.Tag = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_EntityMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

    void Scene::OnRuntimeStart()
    {
        m_IsRunning = true;

        ScriptSystem::Start(this);
        PhysicsSystem::Start(this);
        AnimSystem::Start(this);
        RenderSystem::Start(this);
    }

    void Scene::OnRuntimeStop()
    {
        m_IsRunning = false;

        ScriptSystem::Stop();
        PhysicsSystem::Stop();
        AnimSystem::Stop();
        RenderSystem::Stop();
    }

    void Scene::OnSimulationStart()
    {
        PhysicsSystem::Start(this);
    }

    void Scene::OnSimulationStop()
    {
        PhysicsSystem::Stop();
    }

    void Scene::OnUpdateRuntime(Timestep dt)
    {
        if (!m_IsPaused || m_StepFrames-- > 0)
        {
            ScriptSystem::Update(dt);
            PhysicsSystem::Update(dt);
            AnimSystem::Update(dt);
        }

        RenderSystem::Update(dt);
    }

    void Scene::OnUpdateSimulation(Timestep dt, EditorCamera& camera)
    {
        if (!m_IsPaused || m_StepFrames-- > 0)
        {
            PhysicsSystem::Update(dt);
        }

        // 渲染
        RenderSystem::Update(dt, &camera);
    }

    void Scene::OnUpdateEditor(Timestep dt, EditorCamera& camera)
    {
        // 渲染
        RenderSystem::Start(this);
        RenderSystem::Update(dt, &camera);
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        if (m_ViewportWidth == width && m_ViewportHeight == height)
            return;

        if (width > 0 && height > 0)
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
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        std::string name = entity.GetName();
        Entity newEntity = CreateEntity(name);
        CopyComponentIfExists(AllComponents{}, newEntity, entity);
        return newEntity;
    }

    Entity Scene::GetEntityByUUID(UUID uuid)
    {
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
            return {m_EntityMap.at(uuid), this};
        return {};
    }

    Entity Scene::GetEntityByName(std::string_view name)
    {
        auto view = m_Registry.view<CTag>();
        for (auto entity : view)
        {
            const CTag& tc = view.get<CTag>(entity);
            if (tc.Tag == name)
                return Entity{ entity, this };
        }
        return {};
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

    void Scene::Step(int frames)
    {
        m_StepFrames = frames;
    }

    // 组件添加事件
    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
    }

    template<>
    void Scene::OnComponentAdded<CUuid>(Entity entity, CUuid& component)
    {
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
    void Scene::OnComponentAdded<CAnimPlayer>(Entity entity, CAnimPlayer& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CCircleRenderer>(Entity entity, CCircleRenderer& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CNativeScript>(Entity entity, CNativeScript& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CMonoScript>(Entity entity, CMonoScript& component)
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

    template<>
    void Scene::OnComponentAdded<CCircleCollider2D>(Entity entity, CCircleCollider2D& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CTextRenderer>(Entity entity, CTextRenderer& component)
    {
    }
}