#pragma once

namespace Nous {

    class Timestep
    {
    public:
        Timestep(float time = 0.0f)
            : m_Time(time)
        {
        }

        // 转换成float
        operator float() const { return m_Time; }



        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }
    private:
        float m_Time;
    };

    class Time
    {
    public:
        static Timestep Now() { return m_Instance->GetNowTime(); };
    protected:
        virtual Timestep GetNowTime() = 0;
    private:
        static Time* m_Instance;
    };

}

