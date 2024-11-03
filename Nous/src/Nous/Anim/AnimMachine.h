#pragma once

#include "AnimClip.h"

#include "Nous/Asset/Asset.h"
#include "Nous/Asset/AssetManager.h"

namespace Nous
{
	struct AnimState;

	enum class CompareType : uint16_t
	{
		None = 0,
		Equal,
		NotEqual,
		LessThan,
		GreaterThan,
		NotLessThan,
		NotGreaterThan,
	};

	enum class ValueType : uint8_t
	{
		None = 0,
		Float,
		Bool,
	};

	struct AnimStateCondition
	{
		int StateIndex;
		std::string VarName;
		CompareType Compare;
		ValueType ValueType;
		union 
		{
			float FloatValue;
			bool BoolValue;
		} TargetValue;
	};

	struct AnimState
	{
		AssetHandle Clip;
		std::vector<AnimStateCondition> Conditions;
	};

	class AnimMachineSerializer;

	class AnimMachine : public Asset
	{
	public:
		void SetFloat(UUID entity, const std::string& key, float value);
		void SetBool(UUID entity, const std::string& key, bool value);
		float GetFloat(UUID entity, const std::string& key);
		bool GetBool(UUID entity, const std::string& key);
		Ref<AnimClip> GetCurrentClip(UUID entity);

		void SetCurrentClipIndex(UUID entity, int index);

		virtual AssetType GetType() const { return AssetType::AnimMachine; }
	private:
		void UpdateState(UUID entity, const std::string& key);
	private:
		std::vector<AnimState> m_AllStates;

		struct Current
		{
			std::unordered_map<std::string, float> FloatMap;
			std::unordered_map<std::string, bool> BoolMap;
			int CurrentIndex;
		};
		std::unordered_map<UUID, Current> m_Data;
	public:
		int DefaultIndex = 0;

		friend class AnimMachineSerializer;
		
	};
}

