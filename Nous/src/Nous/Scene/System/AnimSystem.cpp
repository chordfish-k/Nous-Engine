#include "pch.h"
#include "AnimSystem.h"

#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"

#include "Nous/Anim/AnimClip.h"
#include "Nous/Anim/AnimMachine.h"

#include "Nous/Asset/AssetManager.h"

namespace Nous
{
	static Scene* s_Scene = nullptr;

	void AnimSystem::Start(Scene* scene)
	{
		s_Scene = scene;
	}

	void AnimSystem::Update(Timestep dt)
	{
		auto view = s_Scene->GetAllEntitiesWith<CAnimPlayer, CSpriteRenderer>();
		for (auto& ent : view)
		{
			auto [animPlayer, spriteRenderer] = view.get<CAnimPlayer, CSpriteRenderer>(ent);

			Ref<AnimClip> clip;

			if (animPlayer.Type == AssetType::AnimClip)
			{
				clip = AssetManager::GetAsset<AnimClip>(animPlayer.AnimClip);
				
			}
			else if (animPlayer.Type == AssetType::AnimMachine)
			{
				auto& machine = AssetManager::GetAsset<AnimMachine>(animPlayer.AnimClip);
				clip = machine->GetCurrentClip();
			}

			if (!clip)
				continue;

			auto& nowTime = clip->CurrentFrameTimeUsed += dt;
			auto& dur = clip->Frames[clip->CurrentFrame].Duration;
			while (nowTime >= dur)
			{
				clip->CurrentFrameTimeUsed -= dur;
				clip->CurrentFrame++;
				// LOOP
				if (clip->CurrentFrame >= clip->Frames.size())
					clip->CurrentFrame = 0;
			}

			auto& nowFrame = clip->Frames[clip->CurrentFrame];
			if (clip->Type == AnimClipType::Single)
			{
				spriteRenderer.Texture = nowFrame.ImageHandle;
			}
			else
			{
				spriteRenderer.SheetWidth = clip->SheetWidth;
				spriteRenderer.SheetHeight = clip->SheetHeight;
				spriteRenderer.Texture = clip->ImageHandle;
				spriteRenderer.Index = nowFrame.Index;
			}
		}
	}

	void AnimSystem::Stop()
	{
		s_Scene = nullptr;
	}
}