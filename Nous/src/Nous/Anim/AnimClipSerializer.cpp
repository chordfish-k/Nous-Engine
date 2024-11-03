#include "pch.h"
#include "AnimClipSerializer.h"

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
		static std::string AnimClipTypeToString(AnimClipType type)
		{
			switch (type)
			{
			case AnimClipType::Single: return "Single";
			case AnimClipType::SpriteSheet: return "SpriteSheet";
			}
		}

		static AnimClipType StringToAnimClipType(const std::string& type)
		{
			if (type == "Single") return AnimClipType::Single;
			if (type == "SpriteSheet") return AnimClipType::SpriteSheet;
		}
	}

	AnimClipSerializer::AnimClipSerializer(const Ref<AnimClip>& animClip)
		: m_AnimClip(animClip)
	{
	}

	void AnimClipSerializer::Serialize(const std::filesystem::path& filepath)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << m_AnimClip->Name;
		out << YAML::Key << "Type" << YAML::Value << Utils::AnimClipTypeToString(m_AnimClip->Type);
		bool isSingle = m_AnimClip->Type == AnimClipType::Single;
		if (!isSingle)
		{
			out << YAML::Key << "ImageHandle" << YAML::Value << m_AnimClip->ImageHandle;
			out << YAML::Key << "SheetWidth" << YAML::Value << m_AnimClip->SheetWidth;
			out << YAML::Key << "SheetHeight" << YAML::Value << m_AnimClip->SheetHeight;
		}

		out << YAML::Key << "Frame" << YAML::Value << YAML::BeginSeq;
		
		for (auto& frame : m_AnimClip->Frames)
		{
			out << YAML::BeginMap;
			if (isSingle)
				out << YAML::Key << "Handle" << YAML::Value << frame.ImageHandle;
			else
				out << YAML::Key << "Index" << YAML::Value << frame.Index;
			out << YAML::Key << "Duration" << YAML::Value << frame.Duration;
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool AnimClipSerializer::Deserialize(const std::filesystem::path& filepath)
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

		if (!data["Name"])
			return false;
		m_AnimClip->Name = data["Name"].as<std::string>();
		m_AnimClip->Type = Utils::StringToAnimClipType(data["Type"].as<std::string>());

		bool isSingle = m_AnimClip->Type == AnimClipType::Single;
		if (!isSingle)
		{
			m_AnimClip->ImageHandle = data["ImageHandle"].as<AssetHandle>();
			m_AnimClip->SheetWidth = data["SheetWidth"].as<int>();
			m_AnimClip->SheetHeight = data["SheetHeight"].as<int>();
		}

		auto& frames = data["Frames"];
		if (!frames || !frames.IsSequence())
			return false;
		
		for (auto& frame : frames)
		{
			AnimFrame f;
			if (isSingle)
			{
				f.ImageHandle = frame["Handle"].as<AssetHandle>();
			}
			else
			{
				f.Index = frame["Index"].as<int>();
			}
			f.Duration = frame["Duration"].as<float>();
			m_AnimClip->Frames.emplace_back(f);
		}
		return true;
	}
}