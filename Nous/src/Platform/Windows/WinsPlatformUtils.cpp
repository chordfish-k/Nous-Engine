#include "pch.h"
#include "Nous/Utils/PlatformUtils.h"
#include "Nous/Core/Application.h"

#include <commdlg.h> // Windows Only
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Nous {

    float Time::GetTime()
    {
        return glfwGetTime();
    }

    // 选择文件 返回文件名 空字符串代表取消
    std::string FileDialogs::OpenFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        CHAR currentDir[256] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::string();
    }

    // 保存文件 返回文件名 空字符串代表取消
    std::string FileDialogs::SaveFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        CHAR currentDir[256] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        ofn.lpstrDefExt = strchr(filter, '\0') + 1; // 默认选中的扩展名
        if (GetSaveFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::string();
    }

    // 选择创建文件夹
    std::string FileDialogs::CreateFolder()
    {
        // 初始化OPENFILENAME结构体
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        CHAR currentDir[256] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrInitialDir = ".";
        ofn.lpstrFilter = "Folder\0" "New Folder\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

        // 尝试打开文件夹
        if (GetOpenFileNameA(&ofn))
        {
            // 文件夹存在，尝试创建新文件夹
            if (!CreateDirectoryA(ofn.lpstrFile, NULL))
            {
                // 创建文件夹失败，返回错误信息
                DWORD dwError = GetLastError();
                if (dwError != ERROR_ALREADY_EXISTS && dwError != ERROR_FILE_EXISTS)
                {
                    return "Failed to create folder.";
                }
            }
            return ofn.lpstrFile;
        }
        return std::string();
    }
}