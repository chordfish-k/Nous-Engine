#include "pch.h"
#include "AnimMachineImporter.h"

#include "Nous/Anim/AnimMachineSerializer.h"

namespace Nous
{
    Ref<AnimMachine> AnimMachineImporter::ImportAnimMachine(AssetHandle handle, const AssetMetadata& metadata)
    {
        NS_PROFILE_FUNCTION();

        auto res = LoadAnimMachine(Project::GetActiveAssetDirectory() / metadata.FilePath);
        res->Handle = handle;
        return res;
    }

    Ref<AnimMachine> AnimMachineImporter::LoadAnimMachine(const std::filesystem::path& path)
    {
        NS_PROFILE_FUNCTION();

        Ref<AnimMachine> animMachine = CreateRef<AnimMachine>();
        AnimMachineSerializer serializer(animMachine);
        serializer.Deserialize(path);
        return animMachine;
    }

    void AnimMachineImporter::SaveAnimMachine(Ref<AnimMachine> animMachine, const std::filesystem::path& path)
    {
        AnimMachineSerializer serializer(animMachine);
        serializer.Serialize(Project::GetActiveAssetDirectory() / path);
    }
}