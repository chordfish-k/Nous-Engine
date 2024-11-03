#include "pch.h"
#include "AnimMachineSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<Nous::UUID>
	{
		static Node encode(const Nous::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Nous::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

namespace Nous
{
	namespace Utils
	{
		static std::string FloatCompareTypeToString(FloatCompareType type)
		{
			switch (type)
			{
			case FloatCompareType::Equal:			return "eq";
			case FloatCompareType::NotEqual:		return "ne";
			case FloatCompareType::LessThan:		return "lt";
			case FloatCompareType::GreaterThan:		return "gt";
			case FloatCompareType::NotLessThan:		return "gte";
			case FloatCompareType::NotGreaterThan:	return "lte";
			}
			return "";
		}

		static FloatCompareType FloatCompareTypeFromString(const std::string type)
		{
			if (type == "eq")	return FloatCompareType::Equal;
			if (type == "nq")	return FloatCompareType::NotEqual;
			if (type == "lt")	return FloatCompareType::LessThan;
			if (type == "gt")	return FloatCompareType::GreaterThan;
			if (type == "gte")	return FloatCompareType::NotLessThan;
			if (type == "lte")	return FloatCompareType::NotGreaterThan;
			return FloatCompareType::None;
		}

		static std::string BoolCompareTypeToString(BoolCompareType type)
		{
			switch (type)
			{
			case BoolCompareType::And:	return "and";
			case BoolCompareType::Or:	return "or";
			case BoolCompareType::Not:	return "not";
			}
			return "";
		}

		static BoolCompareType BoolCompareTypeFromString(const std::string type)
		{
			if (type == "and")	return BoolCompareType::And;
			if (type == "or")	return BoolCompareType::Or;
			if (type == "not")	return BoolCompareType::Not;
			return BoolCompareType::None;
		}

		static std::string ValueTypeToString(ValueType type)
		{
			switch (type)
			{
			case ValueType::Float:	return "Float";
			case ValueType::Bool:	return "Bool";
			}
			return "";
		}

		static ValueType ValueTypeFromString(const std::string type)
		{
			if (type == "Float")	return ValueType::Float;
			if (type == "Bool")		return ValueType::Bool;
			return ValueType::None;
		}

		static std::string ConditionNodeTypeToString(ConditionNodeType type)
		{
			switch (type)
			{
			case ConditionNodeType::Node:	return "Node";
			case ConditionNodeType::Leaf:	return "Leaf";
			}
			return "";
		}

		static ConditionNodeType ConditionNodeTypeFromString(const std::string type)
		{
			if (type == "Node")	return ConditionNodeType::Node;
			if (type == "Leaf")		return ConditionNodeType::Leaf;
			return ConditionNodeType::None;
		}
	}

	static void SerialzeConditionNode(YAML::Emitter& out, Ref<ConditionNode> node)
	{
		out << YAML::BeginMap;

		out << YAML::Key << "Type" << YAML::Value << Utils::ConditionNodeTypeToString(node->Type);
		if (node->Type == ConditionNodeType::Leaf)
		{
			out << YAML::Key << "VarName" << YAML::Value << node->Exp.VarName;
			out << YAML::Key << "Compare" << YAML::Value << Utils::FloatCompareTypeToString(node->Exp.Compare);
			out << YAML::Key << "ValueType" << YAML::Value << Utils::ValueTypeToString(node->Exp.ValueType);
			if (node->Exp.ValueType == ValueType::Float)
				out << YAML::Key << "TargetValue" << YAML::Value << node->Exp.TargetValue.FloatValue;
			else if (node->Exp.ValueType == ValueType::Bool)
				out << YAML::Key << "TargetValue" << YAML::Value << node->Exp.TargetValue.BoolValue;
		}
		else if (node->Type == ConditionNodeType::Node)
		{
			out << YAML::Key << "Symbol" << YAML::Value << Utils::BoolCompareTypeToString(node->Symbol);
			if (node->LeftChild)
			{
				out << YAML::Key << "Left" << YAML::BeginMap;
				SerialzeConditionNode(out, node->LeftChild);
				out << YAML::EndMap;
			}
			if (node->RightChild)
			{
				out << YAML::Key << "Right" << YAML::BeginMap;
				SerialzeConditionNode(out, node->RightChild);
				out << YAML::EndMap;
			}
		}

		out << YAML::EndMap;
	}

	static Ref<ConditionNode> DeserialzeConditionNode(YAML::Node& data)
	{
		Ref<ConditionNode> node = CreateRef<ConditionNode>();
		node->Type = Utils::ConditionNodeTypeFromString(data["Type"].as<std::string>());
		if (node->Type == ConditionNodeType::Leaf)
		{
			std::string s = data["VarName"].as<std::string>();
			node->Exp.VarName = s;
			node->Exp.Compare = Utils::FloatCompareTypeFromString(data["Compare"].as<std::string>());
			node->Exp.ValueType = Utils::ValueTypeFromString(data["ValueType"].as<std::string>());
			if (node->Exp.ValueType == ValueType::Float)
			{
				node->Exp.TargetValue.FloatValue = data["TargetValue"].as<float>();
			}
			else if (node->Exp.ValueType == ValueType::Bool)
			{
				node->Exp.TargetValue.BoolValue = data["TargetValue"].as<bool>();
			}
		}
		else if (node->Type == ConditionNodeType::Node)
		{
			node->LeftChild = DeserialzeConditionNode(data["Left"]);
			node->RightChild = DeserialzeConditionNode(data["Right"]);
		}
		return node;
	}

	AnimMachineSerializer::AnimMachineSerializer(const Ref<AnimMachine>& animMachine)
		: m_AnimMachine(animMachine)
	{
	}

	void AnimMachineSerializer::Serialize(const std::filesystem::path& filepath)
	{
		YAML::Emitter out;

		out << YAML::Key << "States" << YAML::BeginSeq;
		for (auto& state : m_AnimMachine->m_AllStates)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Clip" << state.Clip;
			out << YAML::Key << "Conditions" << YAML::BeginSeq;

			for (auto& con : state.Conditions)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "NextIndex" << YAML::Value << con.StateIndex;
				out << YAML::Key << "Condition" << YAML::Value;
				SerialzeConditionNode(out, con.ConditionRoot);
				out << YAML::EndMap;
			}
			
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::Key << "DefaultIndex" << YAML::Value << m_AnimMachine->DefaultIndex;
		out << YAML::EndSeq;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool AnimMachineSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException& e)
		{
			NS_CORE_ERROR("无法加载文件 '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		// states
		if (!data["States"] || !data["States"].IsSequence())
			return false;

		for (auto& state : data["States"])
		{
			AnimState st;
			st.Clip = state["Clip"].as<AssetHandle>();

			if (!state["Conditions"])
				return false;

			for (auto& con : state["Conditions"])
			{
				AnimStateCondition asc;
				asc.StateIndex = con["NextIndex"].as<int>();
				asc.ConditionRoot = DeserialzeConditionNode(con["Condition"]);
				
				
				st.Conditions.emplace_back(asc);
			}
			m_AnimMachine->m_AllStates.emplace_back(st);
		}

		m_AnimMachine->DefaultIndex = data["DefaultIndex"].as<int>();

		return true;
	}
}