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

		// ÉèÖÃ³õÊ¼×´Ì¬
		auto view = s_Scene->GetAllEntitiesWith<CAnimPlayer, CSpriteRenderer>();
		for (auto& ent : view)
		{
			auto animPlayer = view.get<CAnimPlayer>(ent);

			if (animPlayer.Type == AssetType::AnimMachine)
			{
				auto& m = AssetManager::GetAsset<AnimMachine>(animPlayer.AnimClip);
				m->SetCurrentClipIndex(Entity{ ent, s_Scene }.GetUUID(), m->DefaultIndex);
			}
		}
	}

	void AnimSystem::Update(Timestep dt)
	{
		auto view = s_Scene->GetAllEntitiesWith<CAnimPlayer, CSpriteRenderer>();
		for (auto& ent : view)
		{
			auto [animPlayer, spriteRenderer] = view.get<CAnimPlayer, CSpriteRenderer>(ent);
			auto uuid = Entity{ ent, s_Scene }.GetUUID();

			Ref<AnimClip> clip;

			if (animPlayer.Type == AssetType::AnimClip)
			{
				clip = AssetManager::GetAsset<AnimClip>(animPlayer.AnimClip);
			}
			else if (animPlayer.Type == AssetType::AnimMachine)
			{
				auto& machine = AssetManager::GetAsset<AnimMachine>(animPlayer.AnimClip);
				clip = machine->GetCurrentClip(uuid);
			}

			if (!clip)
				continue;

			auto& currentFrameTimeUsed = clip->Data[uuid].FrameTimeUsed;
			auto& currentFrame = clip->Data[uuid].Frame;

			auto& nowTime = currentFrameTimeUsed += dt;
			auto& dur = clip->Frames[currentFrame].Duration;
			while (nowTime >= dur)
			{
				currentFrameTimeUsed -= dur;
				currentFrame++;
				// LOOP
				if (currentFrame >= clip->Frames.size())
					currentFrame = clip->Loop ? 0 : currentFrame - 1;
			}

			auto& nowFrame = clip->Frames[currentFrame];
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