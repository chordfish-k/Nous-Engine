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

		struct Current
		{
			int Frame = 0;
			float FrameTimeUsed = 0.0f;
		};
		std::unordered_map<UUID, Current> Data;
		// Single
		std::string Name = "None";
		// SpriteSheet
		AssetHandle ImageHandle = 0;
		int SheetWidth = 0;
		int SheetHeight = 0;

		// 
		bool Loop = true;
	};

	
}