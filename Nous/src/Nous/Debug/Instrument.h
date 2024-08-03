#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

namespace Nous {

    using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

    struct ProfileResult
    {
        std::string Name;

        FloatingPointMicroseconds Start;
        std::chrono::microseconds ElapsedTime;
        std::thread::id ThreadID;
    };

    struct InstrumentationSession
    {
        std::string Name;
    };

    /**
     * 调试工具
     */
    class Instrument
    {
    private:
        std::mutex m_Mutex;
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;

    public:
        Instrument()
            : m_CurrentSession(nullptr)
        {
        }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession)
            {
                // 如果已经有一个 session, 先关闭它再创建新的
                // 用于原始会话的后续分析输出将最终出现在新打开的会话中。这比格式错误的分析输出要好。
                if (Log::GetCoreLogger())
                { // BeginSession() 可能在 Log::Init() 之前执行
                    NS_CORE_ERROR("Instrument::BeginSession('{0}') 但 '{1}' 已开启.", name, m_CurrentSession->Name);
                }
                InternalEndSession();
            }
            m_OutputStream.open(filepath);
            if (m_OutputStream.is_open())
            {
                m_CurrentSession = new InstrumentationSession({name});
                WriteHeader();
            } else
            {
                if (Log::GetCoreLogger())
                { // BeginSession() 可能在 Log::Init() 之前执行
                    NS_CORE_ERROR("Instrument 无法打开结果文件 '{0}'.", filepath);
                }
            }
        }

        void EndSession()
        {
            std::lock_guard lock(m_Mutex);
            InternalEndSession();
        }

        void WriteProfile(const ProfileResult& result)
        {
            std::stringstream json;

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            json << std::setprecision(3) << std::fixed;
            json << ",{";
            json << "\"cat\":\"function\",";
            json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
            json << "\"name\":\"" << name << "\",";
            json << "\"ph\":\"X\",";
            json << "\"pid\":0,";
            json << "\"tid\":" << result.ThreadID << ",";
            json << "\"ts\":" << result.Start.count();
            json << "}";

            std::lock_guard lock(m_Mutex);

            if (m_CurrentSession)
            {
                m_OutputStream << json.str();
                m_OutputStream.flush();
            }
        }

        // 单例
        static Instrument& Get()
        {
            static Instrument instance;
            return instance;
        }

        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
            m_OutputStream.flush();
        }

        void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        // 调用前必须持有锁m_Mutex
        void InternalEndSession()
        {
            if (m_CurrentSession)
            {
                WriteFooter();
                m_OutputStream.close();
                delete m_CurrentSession;
                m_CurrentSession = nullptr;
            }
        }
    };

    class InstrumentationTimer
    {
    public:
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            // high_resolution_clock:高精度时钟，受系统时间变化影响
            // steady_clock:不受系统时间变化影响的时间点，用于测量时间间隔或确保时间序列的一致性
            m_StartTimepoint = std::chrono::steady_clock::now();
        }

        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        void Stop()
        {
            auto endTimepoint = std::chrono::steady_clock::now();
            auto highResStart = FloatingPointMicroseconds{m_StartTimepoint.time_since_epoch()};
            auto elapsedTime =
                std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() -
                std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

            Instrument::Get().WriteProfile({m_Name, highResStart, elapsedTime, std::this_thread::get_id()});
            m_Stopped = true;
        }

    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
    };

    namespace InstrumentUtils {

        template <size_t N>
        struct ChangeResult
        {
            char Data[N];
        };

        template <size_t N, size_t K>
        constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
        {
            ChangeResult<N> result = {};

            size_t srcIndex = 0;
            size_t dstIndex = 0;
            while (srcIndex < N)
            {
                size_t matchIndex = 0;
                while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
                    matchIndex++;
                if (matchIndex == K - 1)
                    srcIndex += matchIndex;
                result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
                srcIndex++;
            }
            return result;
        }
    }
}

#ifndef NS_PROFILE
    #define NS_PROFILE 0
#endif

#if NS_PROFILE
// 确定将使用哪个函数签名宏。 这个宏的选择只有在编译器（或预编译器）开始工作时才会被解决。
// 编辑器中，语法高亮可能会错误地标记了错误的宏。
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
        #define NS_FUNC_SIG __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
        #define NS_FUNC_SIG __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
        #define NS_FUNC_SIG __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define NS_FUNC_SIG __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
        #define NS_FUNC_SIG __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
        #define NS_FUNC_SIG __func__
    #elif defined(__cplusplus) && (__cplusplus >= 201103)
        #define NS_FUNC_SIG __func__
    #else
        #define NS_FUNC_SIG "NS_FUNC_SIG unknown!"
    #endif

    #define NS_PROFILE_BEGIN_SESSION(name, filepath) ::Nous::Instrument::Get().BeginSession(name, filepath)
    #define NS_PROFILE_END_SESSION() ::Nous::Instrument::Get().EndSession()
    #define NS_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Nous::InstrumentUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Nous::InstrumentationTimer timer##line(fixedName##line.Data)
	#define NS_PROFILE_SCOPE_LINE(name, line) NS_PROFILE_SCOPE_LINE2(name, line)
	#define NS_PROFILE_SCOPE(name) NS_PROFILE_SCOPE_LINE(name, __LINE__)
    #define NS_PROFILE_FUNCTION() NS_PROFILE_SCOPE(NS_FUNC_SIG)
#else
#define NS_PROFILE_BEGIN_SESSION(name, filepath)
    #define NS_PROFILE_END_SESSION()
    #define NS_PROFILE_SCOPE(name)
    #define NS_PROFILE_FUNCTION()
#endif
