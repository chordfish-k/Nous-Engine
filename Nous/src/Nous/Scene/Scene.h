#pragma once

#include "Nous/Core/Timestep.h"
#include "Nous/Core/UUID.h"
#include "Nous/Renderer/EditorCamera.h"

#include "entt.hpp"

#include <glm/glm.hpp>

class b2World;

namespace Nous {

    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& name = std::string(), const glm::vec3& position = glm::vec3{0.0f, 0.0f, 0.0f});
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), const glm::vec3& position = glm::vec3{0.0f, 0.0f, 0.0f});
        void DestroyEntity(Entity entity);

        // 暂时
        entt::registry& Reg() { return m_Registry; }

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnUpdateRuntime(Timestep dt);
        void OnUpdateEditor(Timestep dt, EditorCamera& camera);
        void OnViewportResize(uint32_t width, uint32_t height);

        void DuplicateEntity(Entity entity);

        Entity GetPrimaryCameraEntity();
        Entity GetSelectedEntity();
        void SetSelectedEntity(Entity entity);

        template<typename... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }
    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    private:
        entt::registry m_Registry; //注册表=实体上下文，包含所有实体数据
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        b2World* m_PhysicsWorld = nullptr;
        entt::entity m_SelectedEntityID = {};

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };
}

