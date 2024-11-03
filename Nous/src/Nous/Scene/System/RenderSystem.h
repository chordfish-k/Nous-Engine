#pragma once

#include "Nous/Scene/Scene.h"

namespace Nous
{
	class RenderSystem
	{
	public:
		static void Start(Scene* scene);

		static void Update(Timestep dt = 0, EditorCamera* camera = nullptr);

		static void Stop();
	};
}