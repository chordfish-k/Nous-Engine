#pragma once

#include "Nous/Anim/AnimMachine.h"

namespace Nous
{
	class AnimMachineEditorPanel
	{
	public:
		void Open(AssetHandle clipHandle);
		void OnImGuiRender();
	};
}
