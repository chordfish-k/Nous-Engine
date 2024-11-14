#pragma once

#include "Nous/Scene/Scene.h"

namespace Nous
{
	class TransformSystem
	{
	public:
		static void Update(Scene* scene);

		static void SetSubtreeDirty(Scene* scene, entt::entity entity);
	};
}