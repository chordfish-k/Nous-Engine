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

    void ScriptSystem::OnCollision(UUID A, UUID B, const glm::vec2& normal, bool type)
    {
        if (s_Scene)
        {
            auto entityIDA = s_Scene->GetEntityByUUID(A);
            auto entityIDB = s_Scene->GetEntityByUUID(B);
            Entity entityA = { entityIDA, s_Scene };
            Entity entityB = { entityIDB, s_Scene };
            ScriptEngine::OnColliedWith(entityA, B, normal, type);
            ScriptEngine::OnColliedWith(entityB, A, -normal, type);
            
        }
    }
}