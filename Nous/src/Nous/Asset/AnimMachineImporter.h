#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Anim/AnimMachine.h"

namespace Nous
{
	class AnimMachineImporter
	{
	public:
		// AssetMetadata��filepath���������Ŀָ����asset�ļ��е�
		static Ref<AnimMachine> ImportAnimMachine(AssetHandle handle, const AssetMetadata& metadata);

		// ֱ�Ӵ�filesystem�ж�ȡ�ļ���·������ڹ���Ŀ¼
		static Ref<AnimMachine> LoadAnimMachine(const std::filesystem::path& path);

		static void SaveAnimMachine(Ref<AnimMachine> scene, const std::filesystem::path& path);
	};

}

