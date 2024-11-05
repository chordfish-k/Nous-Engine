#pragma once

#include "Nous/Scene/Scene.h"

#include "box2d/b2_world.h"

class b2Contact;

namespace Nous
{
	class PhysicsSystem
	{
	public:
		static void Start(Scene* scene);

		static void Update(Timestep dt);

		static void Stop();
	};

	class ContactListener : public b2ContactListener
	{
	public:
		virtual void BeginContact(b2Contact* contact) override;
		virtual void EndContact(b2Contact* contact) override;
	};
}


