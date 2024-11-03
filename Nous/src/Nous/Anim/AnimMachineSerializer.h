#pragma once

#include "AnimMachine.h"

namespace Nous
{
	class AnimMachineSerializer
	{
    public:
        AnimMachineSerializer(const Ref<AnimMachine>& animMachine);

        void Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);
    private:
        Ref<AnimMachine> m_AnimMachine;
	};
}
