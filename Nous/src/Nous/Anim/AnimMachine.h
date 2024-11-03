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
		void SetFloat(const std::string& key, float value);
		void SetBool(const std::string& key, bool value);
		float GetFloat(const std::string& key);
		bool GetBool(const std::string& key);
		Ref<AnimClip> GetCurrentClip();

		virtual AssetType GetType() const { return AssetType::AnimMachine; }
	private:
		void UpdateState(const std::string& key);
	private:
		std::unordered_map<std::string, float> m_FloatMap;
		std::unordered_map<std::string, bool> m_BoolMap;

		std::vector<AnimState> m_AllStates;

		int m_CurrentIndex = 0;

		friend class AnimMachineSerializer;
	};
}

