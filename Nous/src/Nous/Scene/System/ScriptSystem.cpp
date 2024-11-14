#include "pch.h"
#include "ScriptSystem.h"

#include "Nous/Script/ScriptEngine.h"
#include "Nous/Scene/ScriptableEntity.h"

namespace Nous
{
    static Scene* s_Scene = nullptr;

	void ScriptSystem::Start(Scene* scene)
	{
        s_Scene = scene;
        // 脚本
        if (s_Scene)
        {
            ScriptEngine::OnRuntimeStart(s_Scene);
            // 实例化所有脚本entites

            auto view = s_Scene->GetAllEntitiesWith<CMonoScript>();
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                ScriptEngine::OnCreateEntity(entity);
            }

            // 全部实例化之后再调用 OnStart
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                ScriptEngine::OnStartEntity(entity);
            }
        }
	}

	void ScriptSystem::Update(Timestep dt)
	{
        // 执行脚本更新
        if (s_Scene)
        {
            // C# Entity Update
            auto view = s_Scene->GetAllEntitiesWith<CMonoScript>();
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                ScriptEngine::OnUpdateEntity(entity, dt);
            }

            // 原生脚本
            s_Scene->GetAllEntitiesWith<CNativeScript>().each([=](auto ent, auto& script) {
                // 没有脚本实例就先创建
                if (!script.Instance)
                {
                    script.Instance = script.InitScript();
                    script.Instance->m_Entity = Entity{ ent, s_Scene };
                    script.Instance->OnCreate();
                }

                script.Instance->OnUpdate(dt);
            });
        }
	}

	void ScriptSystem::Stop()
	{
        ScriptEngine::OnRuntimeStop();

        // 执行原生脚本销毁
        {
            s_Scene->GetAllEntitiesWith<CNativeScript>().each([=](auto ent, auto& script) {
                if (script.Instance)
                {
                    script.Instance->OnDestroy();
                    script.DestroyScript(&script);
                }
            });
        }
	}

    void ScriptSystem::OnCollisionPreSolve(void* contactPtr, UUID A, UUID B, glm::vec2& normal)
    {
        if (!s_Scene)
            return;
        auto entityIDA = s_Scene->GetEntityByUUID(A);
        auto entityIDB = s_Scene->GetEntityByUUID(B);
        Entity entityA = { entityIDA, s_Scene };
        Entity entityB = { entityIDB, s_Scene };
        
        ScriptEngine::OnCollisionPreSolve(contactPtr, entityA, B, normal);
        ScriptEngine::OnCollisionPreSolve(contactPtr, entityB, A, -normal);
    }

    void ScriptSystem::OnCollisionPostSolve(void* contactPtr, UUID A, UUID B, glm::vec2& normal)
    {
        if (!s_Scene)
            return;
        auto entityIDA = s_Scene->GetEntityByUUID(A);
        auto entityIDB = s_Scene->GetEntityByUUID(B);
        Entity entityA = { entityIDA, s_Scene };
        Entity entityB = { entityIDB, s_Scene };

        ScriptEngine::OnCollisionPostSolve(contactPtr, entityA, B, normal);
        ScriptEngine::OnCollisionPostSolve(contactPtr, entityB, A, -normal);
    }

    void ScriptSystem::OnCollisionExit(void* contactPtr, UUID A, UUID B)
    {
        if (!s_Scene)
            return;
        auto entityIDA = s_Scene->GetEntityByUUID(A);
        auto entityIDB = s_Scene->GetEntityByUUID(B);
        Entity entityA = { entityIDA, s_Scene };
        Entity entityB = { entityIDB, s_Scene };
        // 物体从物理世界移除时，如果此前处于碰撞状态，这个函数也会触发，所以要先判断实体还在不在
        if (entityA)
            ScriptEngine::OnCollisionExit(contactPtr, entityA, entityB ? B : 0);
        if (entityB)
            ScriptEngine::OnCollisionExit(contactPtr, entityB, entityA ? A : 0);
    }

    void ScriptSystem::OnCollisionEnter(void* contactPtr, UUID A, UUID B, glm::vec2& normal)
    {
        if (!s_Scene)
            return;
        auto entityIDA = s_Scene->GetEntityByUUID(A);
        auto entityIDB = s_Scene->GetEntityByUUID(B);
        Entity entityA = { entityIDA, s_Scene };
        Entity entityB = { entityIDB, s_Scene };
        ScriptEngine::OnCollisionEnter(contactPtr, entityA, B, normal);
        ScriptEngine::OnCollisionEnter(contactPtr, entityB, A, -normal);
    }
}