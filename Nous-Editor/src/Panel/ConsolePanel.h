#pragma once

#include <imgui.h>
#include <vector>
#include <string>

#include "Nous/Event/Observer.h"



namespace Nous
{
	class ConsolePanel : public Observer
	{
	public:
		ConsolePanel();

		void Clear();
		void OnImGuiRender();

		virtual void OnEditorEvent(AppEvent &e) override;
	private:
		void OnConsoleClear(const ConsoleClearEvent& e);
	};
}


