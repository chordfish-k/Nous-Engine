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
        // ��������������Ƴ�ʱ�������ǰ������ײ״̬���������Ҳ�ᴥ��������Ҫ���ж�ʵ�廹�ڲ���
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