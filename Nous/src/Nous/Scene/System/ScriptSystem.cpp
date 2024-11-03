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
}