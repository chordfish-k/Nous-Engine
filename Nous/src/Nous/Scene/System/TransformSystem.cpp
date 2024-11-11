#include "pch.h"
#include "TransformSystem.h"

#include "Nous/Scene/Entity.h"

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
		static const glm::mat4& CalcParentTransform(Scene* scene, Entity entity)
		{
			auto& tr = entity.GetComponent<CTransform>();
			if (tr.HasRigidBody)
				return tr.ParentTransform * tr.GetTransform();
			if (tr.Dirty)
			{
				UUID pid = tr.Parent;
				if (pid == 0)
					return tr.GetTransform();
				Entity pe = scene->GetEntityByUUID(pid);
				tr.ParentTransform = CalcParentTransform(scene, pe);

				tr.Dirty = false;
			}
			return tr.ParentTransform * tr.GetTransform();
		}
	}


	void TransformSystem::Update(Scene* scene)
	{
		if (!scene)
			return;

		for (auto& ent : scene->GetAllEntitiesWith<CTransform>())
		{
			Utils::CalcParentTransform(scene, {ent, scene});
		}

	}

	void TransformSystem::SetSubtreeDirty(Scene* scene, entt::entity entity)
	{
		Entity e{ entity, scene };
		auto& tr = e.GetTransform();
		tr.Dirty = true;

		for (auto& uuid : tr.Children)
		{
			SetSubtreeDirty(scene, scene->GetEntityByUUID(uuid));
		}
	}

}