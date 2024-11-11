#pragma once

#include "Nous/Asset/Asset.h"

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

	struct EntityWrapper
	{
		UUID ID;
	};

	struct AssetHandleWrapper
	{
		uint64_t Handle;

		AssetHandleWrapper() : Handle(0) {}
		AssetHandleWrapper(uint64_t handle) : Handle(handle) {}
		AssetHandleWrapper(UUID handle) : Handle(handle) {}
		operator uint64_t () const { return Handle; }
		operator UUID () const { return Handle; }
	};
}

