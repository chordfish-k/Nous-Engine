#pragma once

namespace Nous
{
	class ScriptGlue
	{
	public:
		static void RegisterComponents();
		static void RegisterFunctions();
	};

	//
	struct ClassWrapper
	{
		void* ptr;
	};
}

