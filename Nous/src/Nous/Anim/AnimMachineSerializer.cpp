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
		static std::string CompareTypeToString(CompareType type)
		{
			switch (type)
			{
			case CompareType::Equal:			return "eq";
			case CompareType::NotEqual:			return "ne";
			case CompareType::LessThan:			return "lt";
			case CompareType::GreaterThan:		return "gt";
			case CompareType::NotLessThan:		return "gte";
			case CompareType::NotGreaterThan:	return "lte";
			}
			return "";
		}

		static CompareType CompareTypeFromString(const std::string type)
		{
			if (type == "eq")	return CompareType::Equal;
			if (type == "nq")	return CompareType::NotEqual;
			if (type == "lt")	return CompareType::LessThan;
			if (type == "gt")	return CompareType::GreaterThan;
			if (type == "gte")	return CompareType::NotLessThan;
			if (type == "lte")	return CompareType::NotGreaterThan;
			return CompareType::None;
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
				out << YAML::Key << "VarName" << YAML::Value << con.VarName;
				out << YAML::Key << "Compare" << YAML::Value << Utils::CompareTypeToString(con.Compare);
				out << YAML::Key << "ValueType" << YAML::Value << Utils::ValueTypeToString(con.ValueType);
				if (con.ValueType == ValueType::Float)
				{
					out << YAML::Key << "TargetValue" << YAML::Value << con.TargetValue.FloatValue;
				}
				else if (con.ValueType == ValueType::Bool)
				{
					out << YAML::Key << "TargetValue" << YAML::Value << con.TargetValue.BoolValue;
				}
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
				asc.VarName = con["VarName"].as<std::string>();
				asc.Compare = Utils::CompareTypeFromString(con["Compare"].as<std::string>());
				asc.ValueType = Utils::ValueTypeFromString(con["ValueType"].as<std::string>());
				if (asc.ValueType == ValueType::Float)
				{
					asc.TargetValue.FloatValue = con["TargetValue"].as<float>();
				}
				else if (asc.ValueType == ValueType::Bool)
				{
					asc.TargetValue.BoolValue = con["TargetValue"].as<bool>();
				}
				st.Conditions.emplace_back(asc);
			}
			m_AnimMachine->m_AllStates.emplace_back(st);
		}

		m_AnimMachine->DefaultIndex = data["DefaultIndex"].as<int>();

		return true;
	}
}