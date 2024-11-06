#pragma once

#include "Nous/Scene/Scene.h"

namespace Nous
{
	class ScriptSystem
	{
	public:
		static void Start(Scene* scene);
		static void Update(Timestep dt);
		static void Stop();
		static void OnPreCollision(void* contactPtr, UUID A, UUID B, glm::vec2& normal);
		static void OnCollision(UUID A, UUID B, glm::vec2& normal, bool type); // type: enter / leave
	};
}


