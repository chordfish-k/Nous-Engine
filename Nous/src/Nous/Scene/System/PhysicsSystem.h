#pragma once

#include "Nous/Scene/Scene.h"


namespace Nous
{
	class PhysicsSystem
	{
	public:
		static void Start(Scene* scene);
		static void Update(Timestep dt);
		static void Stop();

		static void EnableDebugDraw(bool enable);

		static void DisableLastContact();
	};
}


