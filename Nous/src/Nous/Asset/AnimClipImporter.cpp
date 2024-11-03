#include "pch.h"
#include "AnimClipImporter.h"

#include "Nous/Anim/AnimClipSerializer.h"
#include "Nous/Project/Project.h"

namespace Nous
{
	Ref<AnimClip> AnimClipImporter::ImportAnimClip(AssetHandle handle, const AssetMetadata& metadata)
	{
		NS_PROFILE_FUNCTION();

		auto res = LoadAnimClip(Project::GetActiveAssetDirectory() / metadata.FilePath);
		res->Handle = handle;
		return res;
	}

	Ref<AnimClip> AnimClipImporter::LoadAnimClip(const std::filesystem::path& path)
	{
		NS_PROFILE_FUNCTION();

		Ref<AnimClip> animClip = CreateRef<AnimClip>();
		AnimClipSerializer serializer(animClip);
		serializer.Deserialize(path);
		return animClip;
	}

	void AnimClipImporter::SaveAnimClip(Ref<AnimClip> scene, const std::filesystem::path& path)
	{
		AnimClipSerializer serializer(scene);
		serializer.Serialize(Project::GetActiveAssetDirectory() / path);
	}
}