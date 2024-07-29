#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>

namespace Nous {
    struct ProfileResult
    {
        std::string Name;
        long long Start, End;
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
        int m_ProfileCount;
    public:
        Instrument()
            : m_CurrentSession(nullptr)
        {
        }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
//            m_OutputStream.open(filepath);
//            WriteHeader();
//            m_CurrentSession = new InstrumentationSession{ name };

            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession) {
                // 如果已经有一个 session, 先关闭它再创建新的
                // 用于原始会话的后续分析输出将最终出现在新打开的会话中。这比格式错误的分析输出要好。
                if (Log::GetCoreLogger()) { // Edge case: BeginSession() might be before Log::Init()
                    NS_CORE_ERROR("Instrument::BeginSession('{0}') 但 '{1}' 已开启.", name, m_CurrentSession->Name);
                }
                InternalEndSession();
            }
            m_OutputStream.open(filepath);
            if (m_OutputStream.is_open()) {
                m_CurrentSession = new InstrumentationSession({name});
                WriteHeader();
            } else {
                if (Log::GetCoreLogger()) { // Edge case: BeginSession() might be before Log::Init()
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
            std::stringstream  json;

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            json << ",{";
            json << "\"cat\":\"function\",";
            json << "\"dur\":" << (result.End - result.Start) << ',';
            json << "\"name\":\"" << name << "\",";
            json << "\"ph\":\"X\",";
            json << "\"pid\":0,";
            json << "\"tid\":" << result.ThreadID << ",";
            json << "\"ts\":" << result.Start;
            json << "}";

            std::lock_guard lock(m_Mutex);

            if (m_CurrentSession)
            {
                m_OutputStream << json.str();
                m_OutputStream.flush();
            }
        }

        // 单例
        static Instrument& Get() {
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
        void InternalEndSession() {
            if (m_CurrentSession) {
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
            m_StartTimePoint = std::chrono::high_resolution_clock::now();
        }

        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        void Stop()
        {
            auto endTimePoint = std::chrono::high_resolution_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

            Instrument::Get().WriteProfile({ m_Name, start, end, std::this_thread::get_id() });

            m_Stopped = true;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
        bool m_Stopped;
    };
}

#define NS_PROFILE 1
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
    #define NS_PROFILE_SCOPE(name) ::Nous::InstrumentationTimer timer##__LINE__(name)
    #define NS_PROFILE_FUNCTION() NS_PROFILE_SCOPE(NS_FUNC_SIG)
#else
#define NS_PROFILE_BEGIN_SESSION(name, filepath)
	#define NS_PROFILE_END_SESSION()
	#define NS_PROFILE_SCOPE(name)
	#define NS_PROFILE_FUNCTION()
#endif
