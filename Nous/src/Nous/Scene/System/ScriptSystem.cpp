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
        // �ű�
        if (s_Scene)
        {
            ScriptEngine::OnRuntimeStart(s_Scene);
            // ʵ�������нű�entites

            auto view = s_Scene->GetAllEntitiesWith<CMonoScript>();
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                ScriptEngine::OnCreateEntity(entity);
            }

            // ȫ��ʵ����֮���ٵ��� OnStart
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                ScriptEngine::OnStartEntity(entity);
            }
        }
	}

	void ScriptSystem::Update(Timestep dt)
	{
        // ִ�нű�����
        if (s_Scene)
        {
            // C# Entity Update
            auto view = s_Scene->GetAllEntitiesWith<CMonoScript>();
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                ScriptEngine::OnUpdateEntity(entity, dt);
            }

            // ԭ���ű�
            s_Scene->GetAllEntitiesWith<CNativeScript>().each([=](auto ent, auto& script) {
                // û�нű�ʵ�����ȴ���
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

        // ִ��ԭ���ű�����
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

    void ScriptSystem::OnPreCollision(void* contactPtr, UUID A, UUID B, glm::vec2& normal)
    {
        if (!s_Scene)
            return;
        auto entityIDA = s_Scene->GetEntityByUUID(A);
        auto entityIDB = s_Scene->GetEntityByUUID(B);
        Entity entityA = { entityIDA, s_Scene };
        Entity entityB = { entityIDB, s_Scene };
        
        ScriptEngine::OnPreColliedWith(contactPtr, entityA, B, normal);
        ScriptEngine::OnPreColliedWith(contactPtr, entityB, A, -normal);
    }

    void ScriptSystem::OnCollision(UUID A, UUID B, glm::vec2& normal, bool type)
    {
        if (!s_Scene)
            return;
        auto entityIDA = s_Scene->GetEntityByUUID(A);
        auto entityIDB = s_Scene->GetEntityByUUID(B);
        Entity entityA = { entityIDA, s_Scene };
        Entity entityB = { entityIDB, s_Scene };
        ScriptEngine::OnColliedWith(entityA, B, normal, type);
        ScriptEngine::OnColliedWith(entityB, A, -normal, type);
    }
}