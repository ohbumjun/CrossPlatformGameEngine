#pragma once

#include "Hazel/Resource/Image/Texture.h"
#include <filesystem>
#include "Panel/NativePanel.h"

namespace HazelEditor
{

class ContentBrowserPanel : public NativePanel
{
public:
    ContentBrowserPanel();

    void OnImGuiRender();

    inline Hazel::TypeId GetType() const override
    {
        return Hazel::Reflection::GetTypeID<ContentBrowserPanel>();
    }

private:
    std::filesystem::path m_CurrentDirectory;
    Hazel::Ref<Hazel::Texture2D> m_DirectoryIcon;
    Hazel::Ref<Hazel::Texture2D> m_FileIcon;
};

} // namespace HazelEditor