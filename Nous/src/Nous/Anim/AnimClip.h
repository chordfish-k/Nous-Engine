#pragma once

#include "Nous/Asset/Asset.h"

#include <string>
#include <vector>

namespace Nous
{
	enum class AnimClipType : uint8_t
	{
		Single = 0,
		SpriteSheet
	};

	struct AnimFrame
	{
		AssetHandle ImageHandle;
		int Index;
		float Duration;
	};

	class AnimClip : public Asset
	{
	public:
		AnimClip() = default;
		~AnimClip() = default;
		
		virtual AssetType GetType() const { return AssetType::AnimClip; }
	public:
		AnimClipType Type = AnimClipType::Single;
		std::vector<AnimFrame> Frames;
		int CurrentFrame = 0;
		float CurrentFrameTimeUsed = 0;
		// Single
		std::string Name;
		// SpriteSheet
		AssetHandle ImageHandle;
		int SheetWidth;
		int SheetHeight;
	};

	
}