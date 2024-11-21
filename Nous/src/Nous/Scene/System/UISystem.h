#pragma once

#include "Nous/Scene/Scene.h"
#include "Nous/Renderer/Framebuffer.h"

namespace Nous
{
	class UISystem
	{
	public:
		static void Start(Scene* scene);

		static void SetViewport(const glm::vec2& mousePos = glm::vec2(0.0f), const glm::vec2& viewpostLeftTop = glm::vec2(0.0f), const glm::vec2& viewpostSize = glm::vec2(0.0f));

		static void SetFramebuffer(Ref<Framebuffer> framebuffer);

		static void Update(Timestep dt = 0, const glm::vec2& viewpostLeftTop = glm::vec2(0.0f), const glm::vec2& viewpostSize = glm::vec2(0.0f));

		static void Stop();
	};

}

