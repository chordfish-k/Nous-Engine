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
#include "Nous/Scene/System/TransformSystem.h"
#include "Nous/Scene/System/UISystem.h"

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
            auto parent = srcSceneRegistry.get<CTransform>(e).Parent;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;

            if (parent == 0)
            {
                newScene->m_RootEntityMap[uuid] = (entt::entity)newEntity;
            }

            if (selected && e == selected)
                newScene->m_SelectedEntityID = (entt::entity)newEntity;
        }

        // 复制 Component 给新 Registry
        CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

        // 设置dirty
        TransformSystem::SetSubtreeDirty(newScene.get());

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
        // 递归删除子节点
        UUID uid = entity.GetUUID();
        auto& transform = entity.GetTransform();
        for (auto& chUid : transform.Children)
        {
            DestroyEntity(GetEntityByUUID(chUid));
        }

        ScriptEngine::OnDestoryEntity(entity);
        PhysicsSystem::DeleteRigidbody(entity);

        // 从父节点删除自身
        if (transform.Parent)
        {
            auto& pTr = GetEntityByUUID(transform.Parent).GetTransform();
            auto& it = std::find(pTr.Children.begin(), pTr.Children.end(), uid);
            if (it != pTr.Children.end())
            {
                pTr.Children.erase(it);
            }
        }
        else
        {
            m_RootEntityMap.erase(uid);
        }

        m_EntityMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

    void Scene::DestroyEntityAfterUpdate(Entity entity)
    {
        m_EntityToBeDeleted.push_back(entity);
    }

    void Scene::OnRuntimeStart()
    {
        m_IsRunning = true;

        TransformSystem::Update(this);

        ScriptSystem::Start(this);
        PhysicsSystem::Start(this);
        AnimSystem::Start(this);
        UISystem::Start(this);
        RenderSystem::Start(this);
    }

    void Scene::OnRuntimeStop()
    {
        m_IsRunning = false;

        ScriptSystem::Stop();
        PhysicsSystem::Stop();
        AnimSystem::Stop();
        UISystem::Stop();
        RenderSystem::Stop();
    }

    void Scene::OnSimulationStart()
    {
        // 初始化物理系统之前更新各个transform的世界坐标系矩阵
        TransformSystem::Update(this);
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
        // 物理系统更新之后，实际渲染之前，更新各个transform的世界坐标系矩阵
        TransformSystem::Update(this);
        RenderSystem::Update(dt);

        UIRenderSystem::Update(dt);
        UISystem::Update(dt);

        // 删除要删除的entity
        if (!m_EntityToBeDeleted.empty())
        {
            for (auto e : m_EntityToBeDeleted)
            {
                DestroyEntity({e, this});
            }
            m_EntityToBeDeleted.clear();
        }
    }

    void Scene::OnUpdateSimulation(Timestep dt, EditorCamera& camera)
    {
        if (!m_IsPaused || m_StepFrames-- > 0)
        {
            PhysicsSystem::Update(dt);
        }
        TransformSystem::Update(this);
        // 渲染
        RenderSystem::Update(dt, &camera);
        UIRenderSystem::Update(dt);
    }

    void Scene::OnUpdateEditor(Timestep dt, EditorCamera& camera)
    {
        TransformSystem::Update(this);
        // 渲染
        RenderSystem::Start(this);
        RenderSystem::Update(dt, &camera);

        UISystem::Start(this);
        UIRenderSystem::Update(dt);
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
        // TODO 待改进
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


#define NS_COMP_ADD(Comp)       \
    template<>                  \
    void Scene::OnComponentAdded(Entity entity, Comp& component)

#define NS_COMP_REMOVE(Comp)       \
    template<>                  \
    void Scene::OnComponentRemoved(Entity entity, Comp& component)

    // 组件添加事件
    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
    }

    template<typename T>
    void Scene::OnComponentRemoved(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
    }

    // ADD
    NS_COMP_ADD(CCamera)
    {
        if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
            component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }
    NS_COMP_ADD(CUuid) {}
    NS_COMP_ADD(CTransform) {}
    NS_COMP_ADD(CTag) {}
    NS_COMP_ADD(CSpriteRenderer) {}
    NS_COMP_ADD(CAnimPlayer) {}
    NS_COMP_ADD(CCircleRenderer) {}
    NS_COMP_ADD(CNativeScript) {}
    NS_COMP_ADD(CMonoScript) {}
    NS_COMP_ADD(CRigidbody2D) {}
    NS_COMP_ADD(CBoxCollider2D) {}
    NS_COMP_ADD(CCircleCollider2D) {}
    NS_COMP_ADD(CTextRenderer) {}

    NS_COMP_ADD(CUIEventBubble) {}
    NS_COMP_ADD(CUIAnchor) {
        entity.GetTransform().UIComponetFlag |= 1 << 0;
        RenderSystem::ClearAspectCache();
    }
    NS_COMP_ADD(CUIButton) {
        entity.GetTransform().UIComponetFlag |= 1 << 1;
    }
    NS_COMP_ADD(CUIText){
        entity.GetTransform().UIComponetFlag |= 1 << 2;
    }
    NS_COMP_ADD(CUIImage) {
        entity.GetTransform().UIComponetFlag |= 1 << 3;
    }

    // Remove
    NS_COMP_REMOVE(CCamera) {}
    NS_COMP_REMOVE(CUuid) {}
    NS_COMP_REMOVE(CTransform) {}
    NS_COMP_REMOVE(CTag) {}
    NS_COMP_REMOVE(CSpriteRenderer) {}
    NS_COMP_REMOVE(CAnimPlayer) {}
    NS_COMP_REMOVE(CCircleRenderer) {}
    NS_COMP_REMOVE(CNativeScript) {}
    NS_COMP_REMOVE(CMonoScript) {}
    NS_COMP_REMOVE(CRigidbody2D) {}
    NS_COMP_REMOVE(CBoxCollider2D) {}
    NS_COMP_REMOVE(CCircleCollider2D) {}
    NS_COMP_REMOVE(CTextRenderer) {}
    
    NS_COMP_REMOVE(CUIEventBubble) {}
    NS_COMP_REMOVE(CUIAnchor) {
        entity.GetTransform().UIComponetFlag ^= 1 << 0;
        TransformSystem::SetSubtreeDirty(this, entity);
    }
    NS_COMP_REMOVE(CUIButton) {
        entity.GetTransform().UIComponetFlag ^= 1 << 1;
    }
    NS_COMP_REMOVE(CUIText) {
        entity.GetTransform().UIComponetFlag ^= 1 << 2;
    }
    NS_COMP_REMOVE(CUIImage) {
        entity.GetTransform().UIComponetFlag ^= 1 << 3;
    }
}