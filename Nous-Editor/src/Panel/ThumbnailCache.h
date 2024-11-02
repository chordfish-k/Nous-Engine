#pragma once

#include "Nous/Project/Project.h"
#include "Nous/Renderer/Texture.h"

namespace Nous
{
	struct ThumbnailImage
	{
		uint64_t Timestamp; // 最后修改时间
		Ref<Texture2D> Image;
	};

	class ThumbnailCache
	{
	public:
		ThumbnailCache(Ref<Project> project);

		Ref<Texture2D> GetOrCreateThumbnail(const std::filesystem::path& path);
	private:
		Ref<Project> m_Project;

		std::map<std::filesystem::path, ThumbnailImage> m_CacheImages;

		// TEMP 使用序列化
		// std::filesystem::path m_ThumbnaillCachePath;
	};
}
