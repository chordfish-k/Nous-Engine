#pragma once

#include "Nous/Renderer/Font.h"
#include "Nous/Asset/AssetMetadata.h"

namespace Nous
{
	class FontImporter
	{
	public:
		static Ref<Font> ImportFont(AssetHandle handle, const AssetMetadata& metadata);

		static Ref<Font> LoadFont(const std::filesystem::path& path);
	};
}

