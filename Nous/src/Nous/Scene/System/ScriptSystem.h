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
		static void OnCollisionPreSolve(void* contactPtr, UUID A, UUID B, glm::vec2& normal);
		static void OnCollisionPostSolve(void* contactPtr, UUID A, UUID B, glm::vec2& normal);
		static void OnCollisionEnter(void* contactPtr, UUID A, UUID B, glm::vec2& normal);
		static void OnCollisionExit(void* contactPtr, UUID A, UUID B);
	};
}


