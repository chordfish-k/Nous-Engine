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

    bool AnimMachine::SolveConditionNode(UUID entity, Ref<ConditionNode> node)
    {
        auto& cur = m_Data[entity];
        bool res = false;
        if (node->Type == ConditionNodeType::Leaf)
        {
            if (node->Exp.ValueType == ValueType::Float)
            {
                float value = cur.FloatMap[node->Exp.VarName];
                float target = node->Exp.TargetValue.FloatValue;

                switch (node->Exp.Compare)
                {
                case FloatCompareType::Equal:            res = (value == target); break;
                case FloatCompareType::NotEqual:         res = (value != target); break;
                case FloatCompareType::LessThan:         res = (value < target);  break;
                case FloatCompareType::GreaterThan:      res = (value > target);  break;
                case FloatCompareType::NotLessThan:      res = (value >= target); break;
                case FloatCompareType::NotGreaterThan:   res = (value <= target); break;
                }
            }
            else if (node->Exp.ValueType == ValueType::Bool)
            {
                bool value = cur.BoolMap[node->Exp.VarName];
                bool target = node->Exp.TargetValue.BoolValue;

                switch (node->Exp.Compare)
                {
                case FloatCompareType::Equal:            res = (value == target); break;
                case FloatCompareType::NotEqual:         res = (value != target); break;
                }
            }
        }
        else
        {
            if (!node->LeftChild || !node->RightChild)
                return false;
            
            bool resL = SolveConditionNode(entity, node->LeftChild);
            bool resR = SolveConditionNode(entity, node->RightChild);

            switch (node->Symbol)
            {
            case BoolCompareType::And:                  res = (resL && resR); break;
            case BoolCompareType::Or:                   res = (resL || resR); break;
            }
        }   
        
        return res;
    }

    void AnimMachine::UpdateState(UUID entity, const std::string& key)
    {
        auto& cur = m_Data[entity];
        for (auto& con : m_AllStates[cur.CurrentIndex].Conditions)
        {
            if (SolveConditionNode(entity, con.ConditionRoot))
            {
                cur.CurrentIndex = con.StateIndex;
            }
        }
    }
}