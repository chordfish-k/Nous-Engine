#include "pch.h"
#include "FontImporter.h"

#include "Nous/Project/Project.h"

namespace Nous
{
	Ref<Font> FontImporter::ImportFont(AssetHandle handle, const AssetMetadata& metadata)
	{
		NS_PROFILE_FUNCTION();

		auto res = LoadFont(Project::GetActiveAssetDirectory() / metadata.FilePath);
		res->Handle = handle;

		return res;
	}

	Ref<Font> FontImporter::LoadFont(const std::filesystem::path& path)
	{
		NS_PROFILE_FUNCTION();

		return CreateRef<Font>(path);
	}
}