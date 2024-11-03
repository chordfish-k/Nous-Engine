#include "pch.h"
#include "AnimMachine.h"

namespace Nous
{
    void AnimMachine::SetFloat(UUID entity, const std::string& key, float value)
    {
        m_Data[entity].FloatMap[key] = value;
        UpdateState(entity, key);
    }

    void AnimMachine::SetBool(UUID entity, const std::string& key, bool value)
    {
        m_Data[entity].BoolMap[key] = value;
        UpdateState(entity, key);
    }
    float AnimMachine::GetFloat(UUID entity, const std::string& key)
    {
        return m_Data[entity].FloatMap[key];
    }

    bool AnimMachine::GetBool(UUID entity, const std::string& key)
    {
        return m_Data[entity].BoolMap[key];
    }

    Ref<AnimClip> AnimMachine::GetCurrentClip(UUID entity)
    {
        if (!m_AllStates.size())
            return nullptr;
        return AssetManager::GetAsset<AnimClip>(m_AllStates[m_Data[entity].CurrentIndex].Clip);
    };

    void AnimMachine::SetCurrentClipIndex(UUID entity, int index)
    {
        m_Data[entity].CurrentIndex = index;
    }

    void AnimMachine::UpdateState(UUID entity, const std::string& key)
    {
        auto& cur = m_Data[entity];
        for (auto& con : m_AllStates[cur.CurrentIndex].Conditions)
        {
            if (con.VarName != key)
                continue;

            bool shouldChange = false;

            if (con.ValueType == ValueType::Float)
            {
                float value = cur.FloatMap[key];
                float target = con.TargetValue.FloatValue;

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
                bool value = cur.BoolMap[key];
                bool target = con.TargetValue.BoolValue;

                switch (con.Compare)
                {
                case CompareType::Equal:            shouldChange = (value == target); break;
                case CompareType::NotEqual:         shouldChange = (value != target); break;
                }
            }

            if (shouldChange)
            {
                cur.CurrentIndex = con.StateIndex;
            }
        }
    }
}