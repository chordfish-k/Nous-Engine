#pragma once

#include <string>

namespace Nous {

    class FileDialogs
    {
    public:
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter, const std::string& = "");
        static std::string CreateFolder();
    };

    class Time
    {
    public:
        static float GetTime();
    };

}

