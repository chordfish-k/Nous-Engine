#pragma once

#include "Nous/Core/Base.h"
#include "Nous/Renderer/Texture.h"

#include "Nous/Asset/Asset.h"

namespace Nous
{
	struct MSDFData;

	class Font : public Asset
	{
	public:
		Font(const std::filesystem::path& filepath);
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data; }
		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

		const std::filesystem::path& GetFilepath() const { return m_Filepath; }
		
		static Ref<Font> GetDefault();

		static AssetType GetStaticType() { return AssetType::Font; }
		virtual AssetType GetType() const { return GetStaticType(); }
	private:
		MSDFData* m_Data;
		Ref<Texture2D> m_AtlasTexture;

		std::filesystem::path m_Filepath;
	};
}