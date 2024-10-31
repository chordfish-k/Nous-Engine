#include "pch.h"
#include "SceneImporter.h"

#include "Nous/Project/Project.h"
#include "Nous/Scene/SceneSerializer.h"

namespace Nous
{
	Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
	{
		NS_PROFILE_FUNCTION();

		return LoadScene(Project::GetAssetsDirectory() / metadata.FilePath);
	}

	Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path& path)
	{
		NS_PROFILE_FUNCTION();

		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(path);
		return scene;
	}

	void SceneImporter::SaveScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(Project::GetAssetsDirectory() / path);
	}
}