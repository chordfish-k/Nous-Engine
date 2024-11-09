#include "pch.h"
#include "TransformSystem.h"

#include "Nous/Scene/Entity.h"

namespace Nous
{
	namespace Utils
	{
		static void CalcParentTransform(Scene* scene, UUID uuid, const glm::mat4& parent = glm::mat4(1.0f))
		{
			Entity entity = scene->GetEntityByUUID(uuid);
			auto& tr = entity.GetComponent<CTransform>();
			tr.ParentTransform = parent;

			for (auto& ch : tr.Children)
				CalcParentTransform(scene, ch, parent * tr.GetTransform());
		}
	}


	void TransformSystem::Update(Scene* scene)
	{
		if (!scene)
			return;

		for (auto& [uuid, ent] : scene->GetRootEntities())
		{
			Utils::CalcParentTransform(scene, uuid);
		}
	}

}