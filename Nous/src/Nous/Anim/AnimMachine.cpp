#include "pch.h"
#include "AnimMachine.h"

namespace Nous
{
    void AnimMachine::SetFloat(const std::string& key, float value)
    {
        m_FloatMap[key] = value;
        UpdateState(key);
    }

    void AnimMachine::SetBool(const std::string& key, bool value)
    {
        m_BoolMap[key] = value;
        UpdateState(key);
    }
    float AnimMachine::GetFloat(const std::string& key)
    {
        return 0.0f;
    }

    bool AnimMachine::GetBool(const std::string& key)
    {
        return false;
    }

    Ref<AnimClip> AnimMachine::GetCurrentClip()
    {
        if (!m_AllStates.size())
            return nullptr;
        return AssetManager::GetAsset<AnimClip>(m_AllStates[m_CurrentIndex].Clip);
    };

    void AnimMachine::UpdateState(const std::string& key)
    {
        for (auto& con : m_AllStates[m_CurrentIndex].Conditions)
        {
            if (con.VarName != key)
                continue;

            bool shouldChange = false;

            if (con.ValueType == ValueType::Float)
            {
                auto value = m_FloatMap[key];
                auto target = con.TargetValue.FloatValue;

                switch (con.Compare)
                {
                case CompareType::Equal:            shouldChange = (value == target); break;
                case CompareType::NotEqual:         shouldChange = (value != target); break;
                case CompareType::LessThan:         shouldChange = (value <  target); break;
                case CompareType::GreaterThan:      shouldChange = (value >  target); break;
                case CompareType::NotLessThan:      shouldChange = (value >= target); break;
                case CompareType::NotGreaterThan:   shouldChange = (value <= target); break;
                }
            }
            else if (con.ValueType == ValueType::Bool)
            {
                auto value = m_BoolMap[key];
                auto target = con.TargetValue.BoolValue;

                switch (con.Compare)
                {
                case CompareType::Equal:            shouldChange = (value == target); break;
                case CompareType::NotEqual:         shouldChange = (value != target); break;
                }
            }

            if (shouldChange)
            {
                m_CurrentIndex = con.StateIndex;
            }
        }
    }
}