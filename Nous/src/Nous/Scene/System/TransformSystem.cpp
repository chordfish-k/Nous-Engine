#include "pch.h"
#include "TransformSystem.h"

#include "Nous/Scene/Entity.h"

#include "Nous/Scene/System/RenderSystem.h"

namespace Nous
{
	namespace Utils
	{
#if 0
		static void CalcParentTransform(Scene* scene, UUID uuid, const glm::mat4& parent = glm::mat4(1.0f))
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			auto& tr = entity.GetComponent<CTransform>();
			tr.ParentTransform = parent;
			for (auto& ch : tr.Children)
				CalcParentTransform(scene, ch, parent * tr.GetTransform());
		}
#endif
		static const glm::mat4& CalcParentTransform(Scene* scene, entt::entity ent)
		{
			Entity entity{ ent, scene };
			CTransform& tr = entity.GetComponent<CTransform>();

			glm::mat4 uiAnchor(1.0f);

			//?
			if (tr.HasRigidBody)
				return tr.ParentTransform * tr.GetTransform();

			if (tr.Dirty)
			{
				UUID pid = tr.Parent;
				if (pid == 0)
					return tr.GetTransform();

				if (entity.HasComponent<CUIAnchor>())
				{
					auto& ui = entity.GetComponent<CUIAnchor>();
					uiAnchor *= ui.GetTranslate(RenderSystem::GetAspectCache());
				}

				Entity pe = scene->GetEntityByUUID(pid);
				tr.ParentTransform = CalcParentTransform(scene, pe) * uiAnchor;

				tr.Dirty = false;
			}

			return tr.ParentTransform * tr.GetTransform();
		}
	}


	void TransformSystem::Update(Scene* scene)
	{
		NS_PROFILE_FUNCTION();

		if (!scene)
			return;

		auto& view = scene->GetAllEntitiesWith<CTransform, CUuid>();

		for (auto& ent : view)
		{
			Utils::CalcParentTransform(scene, ent);
		}
	}

	void TransformSystem::SetSubtreeDirty(Scene* scene, entt::entity entity)
	{
		if (entity != entt::null)
		{
			Entity e{ entity, scene };
			auto& tr = e.GetTransform();
			tr.Dirty = true;

			for (auto& uuid : tr.Children)
			{
				SetSubtreeDirty(scene, scene->GetEntityByUUID(uuid));
			}
		}
		else
		{
			for (auto& [uid, e] : scene->GetRootEntities())
			{
				TransformSystem::SetSubtreeDirty(scene, e);
			}
		}
	}

	void TransformSystem::SetSubtreeActive(Scene* scene, entt::entity entity, bool active)
	{
		Entity e{ entity, scene };
		auto& tr = e.GetTransform();
		tr.Active = active;

		for (auto& uuid : tr.Children)
		{
			SetSubtreeActive(scene, scene->GetEntityByUUID(uuid), active);
		}
	}

}