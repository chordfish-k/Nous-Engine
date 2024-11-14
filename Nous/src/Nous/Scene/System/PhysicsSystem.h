#pragma once

#include "Nous/Scene/Scene.h"

class b2Body;

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

		static void SetupRigidbody(entt::entity e, b2Body* rootBody = nullptr);
		static void DeleteRigidbody(entt::entity e);
	};
}


