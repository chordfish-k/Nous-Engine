#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Anim/AnimMachine.h"

namespace Nous
{
	class AnimMachineImporter
	{
	public:
		// AssetMetadata的filepath是相对于项目指定的asset文件夹的
		static Ref<AnimMachine> ImportAnimMachine(AssetHandle handle, const AssetMetadata& metadata);

		// 直接从filesystem中读取文件，路径相对于工作目录
		static Ref<AnimMachine> LoadAnimMachine(const std::filesystem::path& path);

		static void SaveAnimMachine(Ref<AnimMachine> scene, const std::filesystem::path& path);
	};

}

