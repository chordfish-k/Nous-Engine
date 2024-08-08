#pragma once

#include <filesystem>

namespace Nous {

    class ResourceBrowserPanel
    {
    public:
        ResourceBrowserPanel();

        void OnImGuiRender();
    private:
        std::filesystem::path m_CurrentDirectory;
    };

}
