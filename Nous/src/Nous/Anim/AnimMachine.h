#pragma once

#include "AnimClip.h"

#include "Nous/Asset/Asset.h"
#include "Nous/Asset/AssetManager.h"

namespace Nous
{
	struct AnimState;

	enum class FloatCompareType : uint16_t
	{
		None = 0,
		Equal,
		NotEqual,
		LessThan,
		GreaterThan,
		NotLessThan,
		NotGreaterThan,
	};

	enum class BoolCompareType : uint16_t
	{
		None = 0,
		And,
		Or,
		Not
	};

	enum class ValueType : uint8_t
	{
		None = 0,
		Float,
		Bool,
	};

	enum class ConditionNodeType : uint8_t
	{
		None = 0,
		Node,
		Leaf
	};

	struct ConditionNode
	{
		ConditionNodeType Type = ConditionNodeType::Leaf;
		struct Exp
		{
			std::string VarName = "";
			FloatCompareType Compare = FloatCompareType::Equal;
			ValueType ValueType = ValueType::Float;
			struct
			{
				float FloatValue = 0.0f;
				bool BoolValue = false;
			} TargetValue;
		} Exp;
		BoolCompareType Symbol = BoolCompareType::And;
		
		Ref<ConditionNode> LeftChild;
		Ref<ConditionNode> RightChild;

		ConditionNode() {}
		~ConditionNode() {}
	};

	struct AnimStateCondition
	{
		int StateIndex;
		Ref<ConditionNode> ConditionRoot;
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

		std::vector<AnimState>& GetAllStates() { return m_AllStates; }

		void SetCurrentClipIndex(UUID entity, int index);

		virtual AssetType GetType() const { return AssetType::AnimMachine; }
	private:
		void UpdateState(UUID entity, const std::string& key);
		bool SolveConditionNode(UUID entity, Ref<ConditionNode> node);
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

