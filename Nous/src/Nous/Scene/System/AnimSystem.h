#pragma once

#include "Nous/Scene/Scene.h"

namespace Nous
{
	class AnimSystem
	{
	public:
		static void Start(Scene* scene);

		static void Update(Timestep dt);

		static void Stop();
	};
}

