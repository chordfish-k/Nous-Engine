#pragma once

#include "Nous/Renderer/Texture.h"

#include <filesystem>
namespace Nous {

    class ResourceBrowserPanel
    {
    public:
        ResourceBrowserPanel();

        void OnImGuiRender();
    private:
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_FileIcon;
    };

}
