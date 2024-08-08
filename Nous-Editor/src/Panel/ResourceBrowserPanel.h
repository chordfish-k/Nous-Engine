#pragma once

#include <filesystem>

#include "Nous/Core/Base.h"
#include "Nous/Renderer/Texture.h"

namespace Nous {

    class ResourceBrowserPanel
    {
    public:
        ResourceBrowserPanel();

        void OnImGuiRender();
    private:
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_FileIcon;
    };

}
