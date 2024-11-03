#pragma once

#include "AnimClip.h"

namespace Nous
{
	class AnimClipSerializer
	{
    public:
        AnimClipSerializer(const Ref<AnimClip>& animClip);

        void Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);
    private:
        Ref<AnimClip> m_AnimClip;
	};
}

