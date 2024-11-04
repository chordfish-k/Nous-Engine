#pragma once

#include "Nous/Anim/AnimClip.h"

namespace Nous
{
	class AnimClipEditorPanel
	{
	public:
		void Open(AssetHandle clipHandle);
		void OnImGuiRender();
	};
}

