#include "ThumbnailCache.h"

#include "Nous/Asset/TextureImporter.h"

#include <chrono>

namespace Nous
{
	ThumbnailCache::ThumbnailCache(Ref<Project> project)
		: m_Project(project) 
	{
		// TODO 移动到cache目录
		// m_ThumbnaillCachePath = m_Project->GetActiveAssetDirectory() / "Thumbnail.cache";
	}

	Ref<Texture2D> ThumbnailCache::GetOrCreateThumbnail(const std::filesystem::path& assetPath)
	{
		// 1. 读时间戳（最后修改时间）
		// 2. 和已经存在的缓存图像比较哈希化时间戳
		// 3. 根据比较情况更新缓存

		auto absolutePath = m_Project->GetAssetAbsolutePath(assetPath);
		if (!std::filesystem::exists(absolutePath))
			return nullptr;

		std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(absolutePath);
		uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();
		
		if (m_CacheImages.find(assetPath) != m_CacheImages.end())
		{
			auto& cachedImage = m_CacheImages.at(assetPath);
			if (cachedImage.Timestamp == timestamp)
				return cachedImage.Image;
		}

		// 缓存不存在就加载
		// 现在只有png
		if (assetPath.extension() != ".png")
			return nullptr;

		Ref<Texture2D> texture = TextureImporter::LoadTexture2D(absolutePath);
		if (!texture)
			return nullptr;

		auto& cacheImage = m_CacheImages[assetPath];
		cacheImage.Timestamp = timestamp;
		cacheImage.Image = texture;
		return cacheImage.Image;
	}
}