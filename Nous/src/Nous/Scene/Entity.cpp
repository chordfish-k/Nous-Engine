#include "pch.h"
#include "Nous/Scene/Entity.h"

namespace Nous {


    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }
}